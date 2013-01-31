using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Diagnostics;

using System.IO;
using Contextify.Shared.Types;

namespace Contextify.Util
{
	public static class Logger
	{
		public delegate void LogEventDelegate(string eventText);
		public static LogEventDelegate LogEventHandler = null;
		public static void LogEvent(string eventText)
		{
			if (LogEventHandler != null)
				LogEventHandler(eventText);
			Trace.WriteLine(eventText);
		}

		public static void LogEvent(string startingText, Exception ex)
		{
			string eventText = startingText + ex.Message + "\n" + ex.StackTrace;
			LogEvent(eventText);
		}


		public static void LogEvent(Exception ex)
		{
			string eventText = ex.Message + "\n" + ex.StackTrace;
			LogEvent(eventText);
		}
	}
}
