using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;

namespace GenLib
{
	public static class Time
	{
		// convert datetime instance to a value that can be used in TNT
		public static UInt64 ToUInt64Time(DateTime time)
		{
			try
			{
				return (UInt64)time.ToFileTimeUtc() / 10000;
			}
			catch { return 0; }		// in case of an invalid time default to 0
		}

		public static double ToUnixTimestamp(DateTime date)
		{
			DateTime origin = new DateTime(1970, 1, 1, 0, 0, 0, 0);
			TimeSpan diff = date - origin;
			return Math.Floor(diff.TotalSeconds);
		}

		//public static double ToUnixTimestamp(this DateTime date)
		//{
		//    DateTime origin = new DateTime(1970, 1, 1, 0, 0, 0, 0);
		//    TimeSpan diff = date - origin;
		//    return Math.Floor(diff.TotalSeconds);
		//}

		// convert a TNT time value to DateTime instance
		public static DateTime FromUInt64Time(UInt64 time)
		{
			return DateTime.FromFileTimeUtc((long)time * 10000);
		}

		public static string ToRelativeTime(this DateTime date)
		{
			DateTime now = DateTime.Now;
			TimeSpan span = now - date;

			// After 7 days show the full date
			if (span.TotalDays > 3)
			{
				if (date.Year != DateTime.Now.Year)
					return date.ToString("ddd d MMM yyyy");
				else
					date.ToString("ddd d MMM");
			}

			if (span < TimeSpan.FromSeconds(0))
			{
				if (span.TotalSeconds < -60)
				{
					return String.Format("in {0} min", Math.Abs(span.TotalMinutes).ToString("##.#"));
				}
				else
				{
					return String.Format("in {0} sec", Math.Abs(span.TotalSeconds).ToString("##"));
				}
			}

			if (span <= TimeSpan.FromSeconds(60))
			{
				return span.Seconds + " sec ago";
			}
			else if (span <= TimeSpan.FromMinutes(60))
			{
				if (span.Minutes > 1)
				{
					return span.Minutes + " min ago";
				}
				else
				{
					return "a minute ago";
				}
			}
			else if (span <= TimeSpan.FromHours(24))
			{
				if (span.Hours > 1)
				{
					return span.Hours + " hrs ago";
				}
				else
				{
					return "an hour ago";
				}
			}
			else
			{
				if (span.Days > 1)
				{
					return span.Days + " days ago";
				}
				else
				{
					return "a day ago";
				}
			}
		}

		/// <summary>
		/// From: http://stackoverflow.com/questions/249760/how-to-convert-unix-timestamp-to-datetime-and-vice-versa
		/// </summary>
		/// <param name="unixTimeStamp"></param>
		/// <returns></returns>
		public static DateTime ToUnixTime(this long unixTimeStamp)
		{
			// Unix timestamp is seconds past epoch
			var epoch = new DateTime(1970, 1, 1, 0, 0, 0, DateTimeKind.Utc);
			return epoch.AddSeconds(unixTimeStamp).ToLocalTime();
		}

		public static long ToUnixTime(this DateTime dateTime)
		{
			var epoch = new DateTime(1970, 1, 1, 0, 0, 0, DateTimeKind.Utc).ToLocalTime();
			var ts = dateTime.Subtract(epoch);

			return ((((((ts.Days * 24) + ts.Hours) * 60) + ts.Minutes) * 60) + ts.Seconds);
		}
	}
}
