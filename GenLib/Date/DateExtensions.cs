namespace GenLib.Date
{
	using System;
	using System.Globalization;
	using System.Runtime.CompilerServices;

	public static class DateExtensions
	{
		private const string DefaultInvalidDateText = "";

		public static bool IsEmpty(this DateTime date)
		{
			return (date == DateTime.MinValue);
		}

		public static bool IsValid(this DateTime date)
		{
			return DateUtilities.IsValid(date);
		}

		public static string ToShortDateOrTimeString(this DateTime date)
		{
			string str = string.Empty;
			if (!date.IsValid())
			{
				return str;
			}
			if (DateUtilities.IsToday(date))
			{
				return date.ToShortTimeString();
			}
			return date.ToShortDateString();
		}

		public static string ToShortDateTimeString(this DateTime date)
		{
			return date.ToShortDateTimeString("");
		}

		public static string ToShortDateTimeString(this DateTime date, string invalidDateText)
		{
			if (!date.IsValid())
			{
				return invalidDateText;
			}
			return ToShortDateTimeStringInternal(date);
		}

		private static string ToShortDateTimeStringInternal(DateTime date)
		{
			return (date.ToShortDateString() + " " + date.ToShortTimeString());
		}

		public static string ToShortMonthDayString(this DateTime date)
		{
			string format = CultureInfo.CurrentCulture.DateTimeFormat.MonthDayPattern.Replace("MMMM", "MMM");
			return date.ToString(format);
		}

		public static string ToShortMonthDayTimeString(this DateTime date)
		{
			return date.ToShortMonthDayTimeString("");
		}

		public static string ToShortMonthDayTimeString(this DateTime date, string invalidDateText)
		{
			if (!date.IsValid())
			{
				return invalidDateText;
			}
			return ToShortMonthDayTimeStringInternal(date);
		}

		private static string ToShortMonthDayTimeStringInternal(DateTime date)
		{
			return (date.ToShortMonthDayString() + " " + date.ToShortTimeString());
		}
	}
}

