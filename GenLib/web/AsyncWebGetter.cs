using System;
using System.Collections.Generic;
using System.IO;
using System.Net;
using System.Windows.Threading;
using System.ComponentModel;
using GenLib.Misc;
using System.Windows.Media;
using GenLib.Text;

namespace GenLib.Web
{
	// a webclient based class that remembers the cookies that are assigned by the visited server
	// and also automatically decompresses the received data stream if it is compressed
	public class CookieAwareWebClient : WebClient
	{
		private readonly CookieContainer m_container = new CookieContainer();

		protected override WebRequest GetWebRequest(Uri address)
		{
			HttpWebRequest webRequest = base.GetWebRequest(address) as HttpWebRequest;
			if (webRequest != null) {
				webRequest.AutomaticDecompression = DecompressionMethods.Deflate | DecompressionMethods.GZip;
				webRequest.CookieContainer = m_container;
			}
			return webRequest;
		}
	}

	// a class that can asynchronously download pages and files from the internet
	public sealed class AsyncWebGetter : IDisposable
	{
		/// <summary>
		/// An AsyncDataRequest that saves the context of the request.
		/// </summary>
		private sealed class FileRequest : Request
		{
			public string DestinationFileName { get; set; }
		}

		private sealed class PageRequest : Request
		{

		}

		private class Request
		{
			public object Sender { get; set; }
			public bool Canceled { get; set; }
			public SmallUri SmallUri { get; set; }
			public object UserState { get; set; }
			public Delegate Callback { get; set; }
		}

		private readonly Queue<Request> _activeWebRequests = new Queue<Request>();
		private readonly Queue<Request> _passiveWebRequests = new Queue<Request>();
		private readonly object _localLock = new object();
		private readonly object _webLock = new object();
		private GenLib.Misc.DispatcherPool _asyncWebRequestPool;
		private QueueEmptyCallback _queueEmptyCallback;
		private bool _disposed = false;

		public string ComputeUrlHash(string url)
		{
			return url.GetMD5HashString();
		}

		public AsyncWebGetter(int threadCount = 1, QueueEmptyCallback queueEmptyCallback = null)
		{
			_queueEmptyCallback = queueEmptyCallback;
			//_asyncWebRequestPool = new DispatcherPool("Web Photo Fetching Thread", threadCount, () => new WebClient { CachePolicy = HttpWebRequest.DefaultCachePolicy });
			_asyncWebRequestPool = new DispatcherPool("Web Getter", threadCount, () => new CookieAwareWebClient());
		}

		public void ClearQueue()
		{
			_activeWebRequests.Clear();
			_passiveWebRequests.Clear();
		}

		#region header functions
		private Dictionary<string, string> _headers = new Dictionary<string, string>();
		public void ClearHeaders()
		{
			_headers = new Dictionary<string, string>();
		}

		// call if you want for example that the webclient represents itself as firefox or IE
		// example values: "user-agent", "Mozilla/4.0 (compatible; MSIE 6.0; Windows NT 5.1; .NET CLR 1.0.3705;)"
		public void AddHeader(string name, string value)
		{
			_headers[name] = value;
		}

		// set the headers that are typically assigned by chrome browser
		public void SetChromBrowserHeaders()
		{
			AddHeader("user-agent", "Mozilla/4.0 (compatible; MSIE 6.0; Windows NT 5.1; .NET CLR 1.0.3705;)");
			AddHeader("Accept", "text/html,application/xhtml+xml,application/xml;q=0.9,*/*;q=0.8");
			AddHeader("Accept-Charset", "ISO-8859-1,utf-8;q=0.7,*;q=0.3");
			AddHeader("Accept-Encoding", "gzip,deflate");
			AddHeader("Accept-Language", "en-GB,en;q=0.8,sl;q=0.6,en-US;q=0.4");
			//AddHeader("Connection", "keep-alive");
		}
		#endregion

		/// <summary>
		/// Download the specified file asynchroniously
		/// </summary>
		/// <param name="sender">sender of the request</param>
		/// <param name="userState">user state - can be null</param>
		/// <param name="ssUri">uri to the file to download</param>
		/// <param name="callback">callback to call when download is complete - can be null if you just want to download it in the background</param>
		/// <param name="pathToStore">directory where the file should be stored</param>
		/// <param name="fileName">filename of the file. If null then MD5 hash is computed from the uri and used as a filename - good to cache images</param>
		public void DownloadFileAsync(object sender, object userState, SmallUri ssUri, SaveFileAsyncCallback callback, string pathToStore, string fileName = null)
		{ 
			if (_disposed)
			{
				callback(this, new SaveFileCompletedEventArgs(new ObjectDisposedException("this"), false, userState));
				return;
			}

			if (default(SmallUri) == ssUri)
			{
				if (callback != null)
					callback(this, new SaveFileCompletedEventArgs(new ArgumentException("The requested image doesn't exist.", "ssUri"), false, userState));
				return;
			}

			if (String.IsNullOrEmpty(fileName))
				fileName = ComputeUrlHash(ssUri.GetString());

			string destinationFullPath = Path.Combine(pathToStore, fileName);
			if (File.Exists(destinationFullPath))
			{
				callback(this, new SaveFileCompletedEventArgs(destinationFullPath, userState, ssUri));
				return;
			}

			// Make asynchronous request to download the image and get the local path.
			var fileRequest = new FileRequest
			{
				Sender = sender,
				UserState = userState,
				SmallUri = ssUri,
				Callback = callback,
				DestinationFileName = Path.Combine(pathToStore, fileName)
			};

			bool needToQueue = false;
			lock (_webLock)
			{
				needToQueue = !_asyncWebRequestPool.HasPendingRequests;
				if (callback != null)
					_activeWebRequests.Enqueue(fileRequest);
				else
					_passiveWebRequests.Enqueue(fileRequest);
			}

			if (needToQueue)
				_asyncWebRequestPool.QueueRequest(_ProcessNextWebRequest, null);
		}

		/// <summary>
		/// Download the web page specified in the ssUri
		/// </summary>
		/// <param name="sender">sender of the request</param>
		/// <param name="userState">user state - can be null</param>
		/// <param name="ssUri">uri to the page to download</param>
		/// <param name="callback">callback to call when download is complete</param>
		public void DownloadPageAsync(object sender, object userState, SmallUri ssUri, PageAsyncCallback callback)
		{
			if (_disposed)
			{
				callback(this, new PageCompletedEventArgs(new ObjectDisposedException("this"), false, userState));
				return;
			}

			if (default(SmallUri) == ssUri)
			{
				if (callback != null)
					callback(this, new PageCompletedEventArgs(new ArgumentException("The requested page doesn't exist.", "ssUri"), false, userState));
				return;
			}

			// Make asynchronous request to download the image and get the local path.
			var pageRequest = new PageRequest
			{
				Sender = sender,
				UserState = userState,
				SmallUri = ssUri,
				Callback = callback
			};

			bool needToQueue = false;
			lock (_webLock)
			{
				needToQueue = !_asyncWebRequestPool.HasPendingRequests;
				_activeWebRequests.Enqueue(pageRequest);
			}

			if (needToQueue)
				_asyncWebRequestPool.QueueRequest(_ProcessNextWebRequest, null);
		}

		private void _ProcessNextWebRequest(object unused)
		{
			if (_asyncWebRequestPool == null)
				return;

			var webClient = _asyncWebRequestPool.Tag as CookieAwareWebClient;
			Assert.IsNotNull(webClient);


			while (_asyncWebRequestPool != null)
			{
				// Retrieve the next data request for processing.
				Request request = null;

				lock (_webLock)
				{
					while (_activeWebRequests.Count > 0)
					{
						request = _activeWebRequests.Dequeue();
						Assert.IsNotNull(request);
						if (!request.Canceled)
							break;
					}

					if (request == null && _passiveWebRequests.Count > 0)
						request = _passiveWebRequests.Dequeue();

					if (request == null)
						return;
				}

				try
				{
					// if we are using special headers then we have to set them for each query and we have to set them here (not before the top lock)
					webClient.Headers.Clear();
					foreach (var key in _headers.Keys)
						webClient.Headers.Add(key, _headers[key]);

					// we are processing a file request
					if (request is FileRequest)
					{
						FileRequest fileRequest = request as FileRequest;
						if (!File.Exists(fileRequest.DestinationFileName))
						{
							// There's a potential race here with other attempts to write the same file.
							// We don't really care because there's not much we can do about it when
							// it happens from multiple processes.
							string tempFile = Path.GetTempFileName();
							Uri address = request.SmallUri.GetUri();
							try
							{
								webClient.DownloadFile(address, tempFile);
							}
							catch (WebException ex)
							{
								// Fail once, just try again.  Servers are flakey.
								// Fails again let it throw.  Caller is expected to catch.
								webClient.DownloadFile(address, tempFile);
							}

							// Should really block multiple web requests for the same file, which causes this...
							if (!GenLib.Utility.TryFileMove(tempFile, fileRequest.DestinationFileName))
							{
								return;
							}
						}
						if (File.Exists(fileRequest.DestinationFileName) && request.Callback != null)
						{
							(fileRequest.Callback as SaveFileAsyncCallback)(this, new SaveFileCompletedEventArgs(fileRequest.DestinationFileName, request.UserState, request.SmallUri));
						}
					}
					// we are processing a page request
					else if (request is PageRequest)
					{
						Uri address = request.SmallUri.GetUri();
						byte[] data = null;
						try
						{
							//System.Diagnostics.Trace.WriteLine(webClient.Headers);
							data = webClient.DownloadData(address);
						}
						catch (WebException ex)
						{
							// Fail once, just try again.  Servers are flakey.
							// Fails again let it throw.  Caller is expected to catch.
							data = webClient.DownloadData(address);
						}

						if (data == null) return;

						System.Text.ASCIIEncoding encoding = new System.Text.ASCIIEncoding();
						string pageContent = encoding.GetString(data);
						if (request.Callback != null)
							(request.Callback as PageAsyncCallback)(this, new PageCompletedEventArgs(pageContent, request.UserState, request.SmallUri));
					}
				}
				catch (Exception ex)
				{ 
					System.Diagnostics.Trace.WriteLine("Error in _ProcessNextWebRequest: " + ex.Message + Environment.NewLine + ex.StackTrace);
				}

				if (_activeWebRequests.Count == 0 && _queueEmptyCallback != null)
				{
					_queueEmptyCallback(this, new QueueEmptyEventArgs());
				}
			}
		}

		internal void Shutdown()
		{
			_disposed = true;
			GenLib.Utility.SafeDispose(ref _asyncWebRequestPool);
		}

		public int ActiveQueueSize
		{
			get
			{
				lock (_webLock)
				{
					return _activeWebRequests.Count;
				}
			}
		}


		#region IDisposable Members

		public void Dispose()
		{
			Shutdown();
		}

		#endregion
	}


	public delegate void SaveFileAsyncCallback(object sender, SaveFileCompletedEventArgs e);

	public class SaveFileCompletedEventArgs : AsyncCompletedEventArgs
	{
		private string _path;
		public SmallUri SmallUri { get; private set; }

		internal SaveFileCompletedEventArgs(string path, object userState, SmallUri uri) : base(null, false, userState)
		{
			Verify.IsNeitherNullNorEmpty(path, "path");
			Assert.IsTrue(File.Exists(path));
			FilePath = path;
			SmallUri = uri;
		}

		/// <summary>
		/// Initializes a new instance of the SaveImageCompletedEventArgs class for an error or a cancellation.
		/// </summary>
		/// <param name="error">Any error that occurred during the asynchronous operation.</param>
		/// <param name="cancelled">A value indicating whether the asynchronous operation was canceled.</param>
		/// <param name="userState">The user-supplied state object.</param>
		internal SaveFileCompletedEventArgs(Exception error, bool cancelled, object userState) : base(error, cancelled, userState)
		{
		}

		public string FilePath
		{
			get
			{
				RaiseExceptionIfNecessary();
				return _path; ;
			}
			private set { _path = value; }
		}
	}

	public delegate void PageAsyncCallback(object sender, PageCompletedEventArgs e);

	public delegate void QueueEmptyCallback(object sender, QueueEmptyEventArgs e);
	
	public class QueueEmptyEventArgs : AsyncCompletedEventArgs
	{
		public QueueEmptyEventArgs()
			: base(null, false, null)
		{ 
		}
	}

	public class PageCompletedEventArgs : AsyncCompletedEventArgs
	{
		public string PageContent { get; private set; }
		public SmallUri SmallUri { get; private set; }

		public PageCompletedEventArgs(string pageContent, object userState, SmallUri uri) : base(null, false, userState)
		{
			PageContent = pageContent;
			SmallUri = uri;
		}

		public PageCompletedEventArgs(Exception error, bool cancelled, object userState)
			: base(error, cancelled, userState)
		{
		}
	}
}
