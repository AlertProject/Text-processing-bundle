
namespace GenLib.Misc
{
	using System;
	using System.Collections.Generic;
	using System.Threading;
	using System.Windows.Threading;

	public class DispatcherPool : IDisposable
	{
		private class _ActionData
		{
			public Action<object> Action;
			public object Arg;
		}

		private readonly object _lock = new object();
		private int _pendingActionsCount = 0;
		private readonly Dispatcher _masterDispatcher;

		private readonly Dictionary<DispatcherPriority, Queue<_ActionData>> _pendingActions = new Dictionary<DispatcherPriority, Queue<_ActionData>>
		{
			{ DispatcherPriority.Background, new Queue<_ActionData>() },
			{ DispatcherPriority.Normal, new Queue<_ActionData>() },
			{ DispatcherPriority.Send, new Queue<_ActionData>() },
		};

		private Dispatcher[] _asyncDispatchers;
		private Dictionary<int, object> _dispatcherTags;
		public int TagCount { get { return _dispatcherTags.Count; } }
		public object GetTag(int index) 
		{ 
			int i=0;
			foreach (var key in _dispatcherTags.Keys)
			{
				if (i == index) return _dispatcherTags[key];
				i++;
			}
			return null; 
		}
		private bool _disposed = false;

		public DispatcherPool(string name, int threadCount)
			: this(name, threadCount, null)
		{}

		/// <summary>
		/// create a pool of threads. When tasks will be added using QueueRequest, the task will be delegated to the free thread and executed
		/// </summary>
		/// <param name="name"></param>
		/// <param name="threadCount"></param>
		/// <param name="tagGenerator"></param>
		public DispatcherPool(string name, int threadCount, Func<object> tagGenerator)
		{
			//Verify.IsNeitherNullNorEmpty(name, "name");
			//Verify.BoundedInteger(1, threadCount, 10, "threadCount");
			_masterDispatcher = Dispatcher.CurrentDispatcher;

			_asyncDispatchers = new Dispatcher[threadCount];
			_dispatcherTags = new Dictionary<int, object>();

			for (int i = 0; i < _asyncDispatchers.Length; ++i)
			{
				var dispatcherThread = new Thread(_DispatcherThreadProc);
				dispatcherThread.SetApartmentState(ApartmentState.STA);
				if (_asyncDispatchers.Length > 1)
					dispatcherThread.Name = name + " (" + (i+1).ToString() + ")";
				else
					dispatcherThread.Name = name;

				using (var dispatcherCreated = new AutoResetEvent(false))
				{
					dispatcherThread.Start(dispatcherCreated);
					dispatcherCreated.WaitOne();
				}

				Dispatcher currentDispatcher = Dispatcher.FromThread(dispatcherThread);
				_asyncDispatchers[i] = currentDispatcher;

				// When the thread that created this starts to shut down, shut down this as well.
				_masterDispatcher.ShutdownStarted += (sender, e) => currentDispatcher.BeginInvokeShutdown(DispatcherPriority.Normal);

				//Assert.IsNotNull(_asyncDispatchers[i]);

				if (tagGenerator != null)
				{
					_dispatcherTags.Add(currentDispatcher.Thread.ManagedThreadId, tagGenerator());
				}
			}
		}

		private static void _DispatcherThreadProc(object handle)
		{
			var d = Dispatcher.CurrentDispatcher;

			((AutoResetEvent)handle).Set();
			try
			{
				Dispatcher.Run();		// we have to do this in a try-except since we get an exception on Mono
			}
			catch (Exception ex)
			{
				System.Diagnostics.Trace.WriteLine("Exception while trying to start a new dispatcher: " + ex.Message);
			}
		}
		
		public void QueueRequest(Action action)
		{
			QueueRequest(DispatcherPriority.Normal, action);
		}

		public void QueueRequest(DispatcherPriority priority, Action action)
		{
			QueueRequest(priority, unused => action(), null);
		}
		
		public void QueueRequest(Action<object> action, object arg)
		{
			QueueRequest(DispatcherPriority.Normal, action, arg);
		}

		public void QueueRequest(DispatcherPriority priority, Action<object> action, object arg)
		{
			_VerifyState();

			//Assert.IsTrue(_pendingActions.ContainsKey(priority), "Attempting to use a DispatcherPriority other than one supported by this class.");

			lock (_lock)
			{
				_pendingActions[priority].Enqueue(new _ActionData { Action = action, Arg = arg });
				++_pendingActionsCount;
			}

			// Queue the request on all of the dispatchers.
			// It's the responsibility of the derived cl-ass to only do the processing once.
			foreach (Dispatcher d in _asyncDispatchers)
			{
#if MONO
				d.BeginInvoke(priority, (Action<DispatcherPriority>)_ProcessNextRequest, priority);
#else
				d.BeginInvoke((Action<DispatcherPriority>)_ProcessNextRequest, priority, priority);
#endif	
			}
		}

		public object Tag
		{
			get
			{
				if (_disposed)
				{
					return null;
				}
				//Assert.IsTrue(_dispatcherTags.ContainsKey(Thread.CurrentThread.ManagedThreadId));
				return _dispatcherTags[Thread.CurrentThread.ManagedThreadId];
			}
		}

		public bool HasPendingRequests { get { return _pendingActionsCount != 0; } }
		
		private void _ProcessNextRequest(DispatcherPriority priority)
		{
			if (Dispatcher.CurrentDispatcher.HasShutdownStarted)
			{
				return;
			}

			_ActionData request = null;
			lock (_lock)
			{
				// There may not be any items left.
				// When there are multiple dispatchers we tell all of them about the item
				// So the first one can try to take it.
				if (_pendingActions[priority].Count > 0)
				{
					request = _pendingActions[priority].Dequeue();
					--_pendingActionsCount;
				}
			}

			if (request != null)
			{
				try
				{
					request.Action(request.Arg);
				}
				catch
				{
					// Don't use this as-is.  The ETW functions used don't work on XP.
					// Either need to update this or re-enable once XP is no longer a supported OS.
					//ETWLogger.EventWriteUnhandledDispatcherPoolExceptionEvent(e.Message, e.StackTrace);

					// Don't let exceptions propagate outside the dispatcher.
					// The Actions should be blocking this from ever happening.
					//Assert.Fail(e.Message);
				}
			}
		}

		private void _VerifyState()
		{
			if (_disposed)
			{
				throw new ObjectDisposedException("this");
			}
		}

		#region IDisposable Members

		public void Dispose()
		{
			if (_disposed)
			{
				return;
			}
			_disposed = true;
			foreach (Dispatcher d in _asyncDispatchers)
			{
#if MONO
				d.InvokeShutdown();
				d.Thread.Abort();
#else
				//d.BeginInvokeShutdown(DispatcherPriority.Send);
				d.InvokeShutdown();
#endif
			}
			foreach (object o in _dispatcherTags.Values)
			{
				var disposable = o as IDisposable;
				Utility.SafeDispose(ref disposable); 
			}
		}

		#endregion
	}

}
