using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Diagnostics;

using System.IO;

namespace GenLib.Log
{
	public static class LogService
	{
		public delegate void LogInfoDelegate(string text);
		public static LogInfoDelegate LogInfoHandler = null;
		public static List<ILogger> Loggers = new List<ILogger>();
		
		public static void AddLogger(ILogger logger)
		{
			Loggers.Add(logger);
		}

		public static void LogInfo(string text)
		{
#if! SILVERLIGHT
			Trace.WriteLine("INFO " + text);
			foreach (var logger in Loggers)
				logger.AddText("INFO " + text);
#endif
		}

		public static void LogWarning(string text)
		{
#if! SILVERLIGHT
			Trace.WriteLine("WARNING " + text);
			foreach (var logger in Loggers)
				logger.AddText("WARNING " + text);
#endif
		}

		public static void LogError(string text)
		{
#if! SILVERLIGHT
			Trace.WriteLine("ERROR " + text);
			foreach (var logger in Loggers)
				logger.AddText("ERROR " + text);
#endif
		}

		public static void LogException(Exception ex)
		{
#if! SILVERLIGHT
			Trace.WriteLine("EXCEPTION " + ex.Message + "\n" + ex.StackTrace);
			foreach (var logger in Loggers)
				logger.AddText("EXCEPTION " + ex.Message + "\n" + ex.StackTrace);
#endif
		}

		public static void LogException(string text, Exception ex)
		{
#if! SILVERLIGHT
			string exData = "";
			if (ex != null)
				exData = ex.Message + "\n" + ex.StackTrace;
			Trace.WriteLine("EXCEPTION " + text + exData);
			foreach (var logger in Loggers)
				logger.AddText("EXCEPTION " + text + exData);
#endif
		}
	}

	public interface ILogger
	{
		void AddText(string text);
	}

	public class EventLogLogger : ILogger
	{
		string _appName = "";

		public EventLogLogger(string applicationName)
		{
			_appName = applicationName;
		}

		public void AddText(string text)
		{
#if! SILVERLIGHT
			EventLog.WriteEntry(_appName, text);
#endif
		}
	}

	public class FileLogger : ILogger	//, IDisposable
	{
		string _lastWriteDate = DateTime.Now.ToShortDateString();
		string _fileName = "";
		string _fileNameBase = "log.txt";
		string _folder = "";

		public FileLogger(string folder, string fileNameBase)
		{
			_fileNameBase = fileNameBase;
			_folder = folder;
			CreateFileName();
		}

		public void CreateFileName()
		{
			try
			{
				if (!Directory.Exists(_folder))
					Directory.CreateDirectory(_folder);

				_lastWriteDate = DateTime.Now.ToShortDateString();
				_fileName = Path.Combine(_folder, DateTime.Now.ToString("yyyy-MM-dd ") + _fileNameBase);			
			}
			catch (Exception ex)
			{
				LogService.LogException("FileLogger. Unable to open file: ", ex);
			}
		}

		public void AddText(string text)
		{
			if (DateTime.Now.ToShortDateString() != _lastWriteDate)
				CreateFileName();

			try
			{
				File.AppendAllText(_fileName, DateTime.Now.ToLongTimeString() + " " + text + Environment.NewLine);
			}
			catch (Exception ex)
			{
				Trace.WriteLine("Unable to log text. " + ex.Message);
			}
		}
	}
}
