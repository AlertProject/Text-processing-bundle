namespace GenLib
{
	using System;
	using System.Diagnostics;

	public static class TraceEx
	{
		public static void WriteLine(string text)
		{
			Trace.WriteLine(text);
		}

		public static void WriteLine(string format, object arg0)
		{
			Trace.WriteLine(string.Format(format, arg0));
		}

		public static void WriteLine(string format, params object[] args)
		{
			Trace.WriteLine(string.Format(format, args));
		}

		public static void WriteLine(string format, object arg0, object arg1)
		{
			Trace.WriteLine(string.Format(format, arg0, arg1));
		}

		public static void WriteLine(string format, object arg0, object arg1, object arg2)
		{
			Trace.WriteLine(string.Format(format, arg0, arg1, arg2));
		}
	}
}

