namespace GenLib.Date
{
    using System;

    public static class DateUtilities
    {
        private const string DateRangeFormat = "{0} - {1}";
        private const string DateRangeSameDayFormat = "{0} {1} - {2}";
        public const int DaysInWeek = 7;
        public static int[] HoursRanges = new int[] { 1, 2, 4, 8 };
        public const string InvalidDateText = "[Date not available]";
        public const long InvalidDateTickCount = 0x13b5136ec6544000L;
        public static readonly int MaximumHoursGranularity = HoursRanges[HoursRanges.Length - 1];
        public static readonly int MaximumMinutesGranularity = MinutesRanges[MinutesRanges.Length - 1];
        public static int[] MinutesRanges = new int[] { 15, 0x2d };
        public const int MonthsInYear = 12;
        public static readonly TimeSpan OneDay = new TimeSpan(1, 0, 0, 0);
        public const string StringDaysAfterFormat = "In {0} days";
        public const string StringDaysBeforeFormat = "{0} days ago";
        public const string StringFriday = "Friday";
        public const string StringHoursAfterFormat = "In {0} hours";
        public const string StringHoursAfterSingularFormat = "In {0} hour";
        public const string StringHoursBeforeFormat = "{0} hours ago";
        public const string StringHoursBeforeSingularFormat = "{0} hour ago";
        public const string StringHoursRangeAfterFormat = "Next {0} hours";
        public const string StringHoursRangeAfterSingularFormat = "Next {0} hour";
        public const string StringHoursRangeBeforeFormat = "Last {0} hours";
        public const string StringHoursRangeBeforeSingularFormat = "Last {0} hour";
        public const string StringLastMonth = "Last month";
        public const string StringLastWeek = "Last week";
        public const string StringLastYear = "Last year";
        public const string StringMinutesAfterFormat = "In {0} minutes";
        public const string StringMinutesAfterSingularFormat = "In {0} minutes";
        public const string StringMinutesBeforeFormat = "{0} minutes ago";
        public const string StringMinutesBeforeSingularFormat = "{0} minute ago";
        public const string StringMinutesRangeAfterFormat = "Next {0} minutes";
        public const string StringMinutesRangeAfterSingularFormat = "Next {0} minute";
        public const string StringMinutesRangeBeforeFormat = "Last {0} minutes";
        public const string StringMinutesRangeBeforeSingularFormat = "Last {0} minute";
        public const string StringMonday = "Monday";
        public const string StringMonthsAfterFormat = "In {0} months";
        public const string StringMonthsBeforeFormat = "{0} months ago";
        public const string StringNextMonth = "Next month";
        public const string StringNextWeek = "Next week";
        public const string StringNextYear = "Next year";
        public const string StringNow = "Just now";
        public const string StringSaturday = "Saturday";
        public const string StringSunday = "Sunday";
        public const string StringThisMonth = "This month";
        public const string StringThisWeek = "This weeks";
        public const string StringThisYear = "This year";
        public const string StringThursday = "Thursday";
        public const string StringToday = "Today";
        public const string StringTomorrow = "Tomorrow";
        public const string StringTuesday = "Tuesday";
        public const string StringWednesday = "Wednesday";
        public const string StringWeeksAfterFormat = "In {0} weeks";
        public const string StringWeeksBeforeFormat = "{0} weeks ago";
        public const string StringYearsAfterFormat = "In {0} years";
        public const string StringYearsBeforeFormat = "{0} years ago";
        public const string StringYesterday = "Yesterday";
        public static readonly TimeSpan ThreeDays = new TimeSpan(3, 0, 0, 0);
        public const int WeeksInMonth = 5;

        public static string DateAsRelative(DateTime date)
        {
            return DateAsRelative(date, DateTime.Now, Granularity.Days);
        }

        public static string DateAsRelative(DateTime date, Granularity granularity)
        {
            return DateAsRelative(date, DateTime.Now, granularity);
        }

        public static string DateAsRelative(DateTime date, DateTime baseDate)
        {
            return DateAsRelative(date, baseDate, Granularity.Days);
        }

        public static string DateAsRelative(DateTime date, DateTime baseDate, Granularity granularity)
        {
            int secondsDelta = SecondsDelta(date, baseDate);
            int minutesDelta = MinutesDelta(date, baseDate);
            int hoursDelta = HoursDelta(date, baseDate);
            DaysDelta(date, baseDate);
            int num4 = WeeksDelta(date, baseDate);
            int num5 = MonthsDelta(date, baseDate);
            int delta = YearsDelta(date, baseDate);
            if ((granularity == Granularity.Minutes) && (minutesDelta <= MaximumMinutesGranularity))
            {
                return MinutesDeltaToString(minutesDelta, secondsDelta);
            }
            if (((granularity == Granularity.Minutes) || (granularity == Granularity.Hours)) && (hoursDelta <= MaximumHoursGranularity))
            {
                return HoursDeltaToString(hoursDelta, minutesDelta);
            }
            if (IsToday(date, baseDate))
            {
                return "Today";
            }
            if (IsTomorrow(date, baseDate))
            {
                return "Tomorrow";
            }
            if (IsYesterday(date, baseDate))
            {
                return "Yesterday";
            }
            if (IsThisWeek(date, baseDate))
            {
                return DateToDayOfWeekString(date);
            }
            if (IsLastWeek(date, baseDate))
            {
                return "Last week";
            }
            if (IsNextWeek(date, baseDate))
            {
                return "Next week";
            }
            if (Math.Abs(num4) <= 5)
            {
                return WeeksDeltaToString(num4);
            }
            if (IsLastMonth(date, baseDate))
            {
                return "Last month";
            }
            if (IsNextMonth(date, baseDate))
            {
                return "Next month";
            }
            if (Math.Abs(num5) < 12)
            {
                return MonthsDeltaToString(num5);
            }
            if (IsLastYear(date, baseDate))
            {
                return "Last year";
            }
            if (IsNextYear(date, baseDate))
            {
                return "Next year";
            }
            return YearsDeltaToString(delta);
        }

        public static string DateToDayOfWeekString(DateTime date)
        {
            return DayOfWeekToString(date.DayOfWeek);
        }

        public static string DayOfWeekToString(DayOfWeek dayOfWeek)
        {
            string str = string.Empty;
            switch (dayOfWeek)
            {
                case DayOfWeek.Sunday:
                    return "Sunday";

                case DayOfWeek.Monday:
                    return "Monday";

                case DayOfWeek.Tuesday:
                    return "Tuesday";

                case DayOfWeek.Wednesday:
                    return "Wednesday";

                case DayOfWeek.Thursday:
                    return "Thursday";

                case DayOfWeek.Friday:
                    return "Friday";

                case DayOfWeek.Saturday:
                    return "Saturday";
            }
            return str;
        }

        public static int DaysDelta(DateTime date)
        {
            return DaysDelta(date, DateTime.Today);
        }

        public static int DaysDelta(DateTime date, DateTime baseDate)
        {
            return baseDate.Subtract(date).Days;
        }

        public static string DaysDeltaToString(int delta)
        {
            if (delta == 0)
            {
                return "Today";
            }
            if (delta > 0)
            {
                return string.Format("{0} days ago", Math.Abs(delta));
            }
            return string.Format("In {0} days", Math.Abs(delta));
        }

        public static DateTime FirstDayOfMonth(DateTime date)
        {
            DateTime time = NormalizeToDays(date);
            return new DateTime(time.Year, time.Month, 1);
        }

        public static DateTime FirstDayOfWeek(DateTime date)
        {
            DateTime time = NormalizeToDays(date);
            return time.AddDays(-(int) time.DayOfWeek);
			//return time.AddDays((double)(~DayOfWeek.Sunday * time.DayOfWeek));
        }

        public static DateTime FirstDayOfYear(DateTime date)
        {
            DateTime time = NormalizeToDays(date);
            return new DateTime(time.Year, 1, 1);
        }

        public static int HoursDelta(DateTime date)
        {
            return HoursDelta(date, DateTime.Today);
        }

        public static int HoursDelta(DateTime date, DateTime baseDate)
        {
            return (int) baseDate.Subtract(date).TotalHours;
        }

        public static string HoursDeltaToString(int hoursDelta, int minutesDelta)
        {
            string str = string.Empty;
            int num = Math.Abs(hoursDelta);
            foreach (int num2 in HoursRanges)
            {
                if (num <= num2)
                {
                    if (minutesDelta > 0)
                    {
                        string str2 = (num2 == 1) ? "Last {0} hour" : "Last {0} hours";
                        str = string.Format(str2, num2);
                    }
                    else
                    {
                        string str3 = (num2 == 1) ? "Next {0} hour" : "Next {0} hours";
                        str = string.Format(str3, num2);
                    }
                    break;
                }
            }
            if (!string.IsNullOrEmpty(str))
            {
                return str;
            }
            if (minutesDelta == 0)
            {
                return "Just now";
            }
            if (minutesDelta > 0)
            {
                string str4 = (num == 1) ? "{0} hour ago" : "{0} hours ago";
                return string.Format(str4, num);
            }
            string format = (num == 1) ? "In {0} hour" : "In {0} hours";
            return string.Format(format, num);
        }

        private static bool IsDateInRange(DateTime date, DateTime startDate, DateTime endDate)
        {
            return ((date >= startDate) && (date < endDate));
        }

        public static bool IsLastMonth(DateTime date)
        {
            return IsLastMonth(date, DateTime.Today);
        }

        public static bool IsLastMonth(DateTime date, DateTime baseDate)
        {
            baseDate = FirstDayOfMonth(baseDate);
            baseDate = baseDate.AddDays(-1.0);
            DateTime startDate = FirstDayOfMonth(baseDate);
            DateTime endDate = LastDayOfMonth(baseDate);
            return IsDateInRange(date, startDate, endDate);
        }

        public static bool IsLastWeek(DateTime date)
        {
            return IsLastWeek(date, DateTime.Today);
        }

        public static bool IsLastWeek(DateTime date, DateTime baseDate)
        {
            DateTime startDate = FirstDayOfWeek(baseDate).AddDays(-7.0);
            DateTime endDate = startDate.AddDays(7.0);
            return IsDateInRange(date, startDate, endDate);
        }

        public static bool IsLastYear(DateTime date)
        {
            return IsLastYear(date, DateTime.Today);
        }

        public static bool IsLastYear(DateTime date, DateTime baseDate)
        {
            baseDate = FirstDayOfYear(baseDate);
            baseDate = baseDate.AddDays(-1.0);
            DateTime startDate = FirstDayOfYear(baseDate);
            DateTime endDate = LastDayOfYear(baseDate);
            return IsDateInRange(date, startDate, endDate);
        }

        public static bool IsNextMonth(DateTime date)
        {
            return IsNextMonth(date, DateTime.Today);
        }

        public static bool IsNextMonth(DateTime date, DateTime baseDate)
        {
            baseDate = LastDayOfMonth(baseDate);
            baseDate = baseDate.AddDays(1.0);
            DateTime startDate = FirstDayOfMonth(baseDate);
            DateTime endDate = LastDayOfMonth(baseDate);
            return IsDateInRange(date, startDate, endDate);
        }

        public static bool IsNextWeek(DateTime date)
        {
            return IsNextWeek(date, DateTime.Today);
        }

        public static bool IsNextWeek(DateTime date, DateTime baseDate)
        {
            DateTime startDate = FirstDayOfWeek(baseDate).AddDays(7.0);
            DateTime endDate = startDate.AddDays(7.0);
            return IsDateInRange(date, startDate, endDate);
        }

        public static bool IsNextYear(DateTime date)
        {
            return IsNextYear(date, DateTime.Today);
        }

        public static bool IsNextYear(DateTime date, DateTime baseDate)
        {
            baseDate = LastDayOfYear(baseDate);
            baseDate = baseDate.AddDays(1.0);
            DateTime startDate = FirstDayOfYear(baseDate);
            DateTime endDate = LastDayOfYear(baseDate);
            return IsDateInRange(date, startDate, endDate);
        }

        public static bool IsThisMonth(DateTime date)
        {
            return IsThisMonth(date, DateTime.Today);
        }

        public static bool IsThisMonth(DateTime date, DateTime baseDate)
        {
            DateTime startDate = FirstDayOfMonth(baseDate);
            DateTime endDate = LastDayOfMonth(baseDate);
            return IsDateInRange(date, startDate, endDate);
        }

        public static bool IsThisWeek(DateTime date)
        {
            return IsThisWeek(date, DateTime.Today);
        }

        public static bool IsThisWeek(DateTime date, DateTime baseDate)
        {
            DateTime startDate = FirstDayOfWeek(baseDate);
            DateTime endDate = startDate.AddDays(7.0);
            return IsDateInRange(date, startDate, endDate);
        }

        public static bool IsThisYear(DateTime date)
        {
            return IsThisYear(date, DateTime.Today);
        }

        public static bool IsThisYear(DateTime date, DateTime baseDate)
        {
            DateTime startDate = FirstDayOfYear(baseDate);
            DateTime endDate = LastDayOfYear(baseDate);
            return IsDateInRange(date, startDate, endDate);
        }

        public static bool IsToday(DateTime date)
        {
            return IsToday(date, DateTime.Today);
        }

        public static bool IsToday(DateTime date, DateTime baseDate)
        {
            DateTime startDate = NormalizeToDays(baseDate);
            DateTime endDate = startDate.AddDays(1.0);
            return IsDateInRange(date, startDate, endDate);
        }

        public static bool IsTomorrow(DateTime date)
        {
            return IsTomorrow(date, DateTime.Today);
        }

        public static bool IsTomorrow(DateTime date, DateTime baseDate)
        {
            DateTime startDate = NormalizeToDays(baseDate).AddDays(1.0);
            DateTime endDate = startDate.AddDays(1.0);
            return IsDateInRange(date, startDate, endDate);
        }

        public static bool IsValid(DateTime date)
        {
            return (date.Ticks != 0x13b5136ec6544000L);
        }

        public static bool IsYesterday(DateTime date)
        {
            return IsYesterday(date, DateTime.Today);
        }

        public static bool IsYesterday(DateTime date, DateTime baseDate)
        {
            DateTime startDate = NormalizeToDays(baseDate).AddDays(-1.0);
            DateTime endDate = startDate.AddDays(1.0);
            return IsDateInRange(date, startDate, endDate);
        }

        public static DateTime LastDayOfMonth(DateTime date)
        {
            DateTime time = FirstDayOfMonth(date);
            int num = DateTime.DaysInMonth(date.Year, date.Month);
            return time.AddDays((double) num);
        }

        public static DateTime LastDayOfYear(DateTime date)
        {
            DateTime time = new DateTime(date.Year + 1, 1, 1);
            return time.AddDays(-1.0);
        }

        public static int MinutesDelta(DateTime date)
        {
            return MinutesDelta(date, DateTime.Today);
        }

        public static int MinutesDelta(DateTime date, DateTime baseDate)
        {
            return (int) baseDate.Subtract(date).TotalMinutes;
        }

        public static string MinutesDeltaToString(int minutesDelta, int secondsDelta)
        {
            string str = string.Empty;
            int num = Math.Abs(minutesDelta);
            foreach (int num2 in MinutesRanges)
            {
                if (num <= num2)
                {
                    if (secondsDelta > 0)
                    {
                        string str2 = (num2 == 1) ? "Last {0} minute" : "Last {0} minutes";
                        str = string.Format(str2, num2);
                    }
                    else
                    {
                        string str3 = (num2 == 1) ? "Next {0} minute" : "Next {0} minutes";
                        str = string.Format(str3, num2);
                    }
                    break;
                }
            }
            if (!string.IsNullOrEmpty(str))
            {
                return str;
            }
            if (minutesDelta == 0)
            {
                return "Just now";
            }
            if (minutesDelta > 0)
            {
                string str4 = (num == 1) ? "{0} minute ago" : "{0} minutes ago";
                return string.Format(str4, num);
            }
            string format = (num == 1) ? "In {0} minutes" : "In {0} minutes";
            return string.Format(format, num);
        }

        public static int MonthsDelta(DateTime date)
        {
            return MonthsDelta(date, DateTime.Today);
        }

        public static int MonthsDelta(DateTime date, DateTime baseDate)
        {
            return (((baseDate.Year * 12) + baseDate.Month) - ((date.Year * 12) + date.Month));
        }

        public static string MonthsDeltaToString(int delta)
        {
            if (delta == 0)
            {
                return "This month";
            }
            if (delta > 0)
            {
                return string.Format("{0} months ago", Math.Abs(delta));
            }
            return string.Format("In {0} months", Math.Abs(delta));
        }

        public static DateTime NormalizeToDays(DateTime date)
        {
            return new DateTime(date.Year, date.Month, date.Day);
        }

        public static string RangeToString(DateTime startDate, DateTime endDate)
        {
            if (startDate.Date == endDate.Date)
            {
                return string.Format("{0} {1} - {2}", startDate.Date.ToShortDateString(), startDate.ToShortTimeString(), endDate.ToShortTimeString());
            }
            return string.Format("{0} - {1}", startDate.ToShortDateTimeString(), endDate.ToShortDateTimeString());
        }

        public static int SecondsDelta(DateTime date)
        {
            return SecondsDelta(date, DateTime.Today);
        }

        public static int SecondsDelta(DateTime date, DateTime baseDate)
        {
            return (int) baseDate.Subtract(date).TotalSeconds;
        }

        public static int WeeksDelta(DateTime date)
        {
            return WeeksDelta(date, DateTime.Today);
        }

        public static int WeeksDelta(DateTime date, DateTime baseDate)
        {
            DateTime time = FirstDayOfWeek(date);
            return (FirstDayOfWeek(baseDate).Subtract(time).Days / 7);
        }

        public static string WeeksDeltaToString(int delta)
        {
            if (delta == 0)
            {
                return "This weeks";
            }
            if (delta > 0)
            {
                return string.Format("{0} weeks ago", Math.Abs(delta));
            }
            return string.Format("In {0} weeks", Math.Abs(delta));
        }

        public static int YearsDelta(DateTime date)
        {
            return YearsDelta(date, DateTime.Today);
        }

        public static int YearsDelta(DateTime date, DateTime baseDate)
        {
            return (baseDate.Year - date.Year);
        }

        public static string YearsDeltaToString(int delta)
        {
            if (delta == 0)
            {
                return "This year";
            }
            if (delta > 0)
            {
                return string.Format("{0} years ago", Math.Abs(delta));
            }
            return string.Format("In {0} years", Math.Abs(delta));
        }

        public enum Granularity
        {
            Minutes,
            Hours,
            Days
        }
    }
}

