using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Text.RegularExpressions;
using System.Diagnostics.CodeAnalysis;
using System.Security.Cryptography;
using System.Globalization;

namespace GenLib.Text
{
	public static class StringExtension
	{
		/// <summary>
		/// encode the string to be valid to be put into the xml
		/// </summary>
		/// <param name="value"></param>
		/// <returns></returns>
		static public string EncodeXMLString(this string value)
		{
			if (string.IsNullOrEmpty(value)) return value;
			value = value.Replace("&", "&amp;");
			value = value.Replace("<", "&lt;");
			value = value.Replace(">", "&gt;");
			value = value.Replace("\"", "&quot;");
			value = value.Replace("'", "&apos;");
			//value = EncodeNonAsciiCharacters(value);
			return value;
		}

		/// <summary>
		/// decode the string that was read from an xml. replace &lt; with <, etc
		/// </summary>
		/// <param name="value"></param>
		/// <returns></returns>
		static public string DecodeXMLString(this string value)
		{
			if (string.IsNullOrEmpty(value)) return value;
			value = Text.DecodeNonAsciiCharacters(value);
			value = value.Replace("&quot;", "\"");
			value = value.Replace("&apos;", "'");
			value = value.Replace("&lt;", "<");
			value = value.Replace("&gt;", ">");
			value = value.Replace("&amp;", "&");
			return value;
		}

#if! SILVERLIGHT
		// capitalize each word in the string
		public static string CapitalizeWords(this string name)
		{
			if (string.IsNullOrEmpty(name)) return name;
			return CultureInfo.CurrentCulture.TextInfo.ToTitleCase(name);
		}


		/// <summary>
		/// replace unicode characters with their ascii approximations - replace ščć with scc and so on
		/// </summary>
		/// <param name="text"></param>
		/// <returns></returns>
		public static string ReplaceUnicodeCharsWithAscii(this string text)
		{
			if (string.IsNullOrEmpty(text)) return text;
			string normalized = text.Normalize(NormalizationForm.FormKD);
			Encoding ascii = Encoding.GetEncoding("us-ascii", new EncoderReplacementFallback(string.Empty), new DecoderReplacementFallback(string.Empty));
			byte[] encodedBytes = new byte[ascii.GetByteCount(normalized)];
			int numberOfEncodedBytes = ascii.GetBytes(normalized, 0, normalized.Length, encodedBytes, 0);
			return ascii.GetString(encodedBytes);
		}
#endif 

		/// <summary>
		/// count the number of times the specified char appears
		/// </summary>
		/// <param name="text"></param>
		/// <param name="c"></param>
		/// <returns></returns>
		public static int CountChar(this string text, char c)
		{
			if (string.IsNullOrEmpty(text)) return 0;
			return text.Count(ch => ch == c);
			//int result = 0;
			//foreach (char curChar in instance)
			//{
			//    if (c == curChar)
			//        ++result;
			//}
			//return result;
		}

		public static string StripHtml(this string source)
		{
			if (String.IsNullOrEmpty(source))
				return String.Empty;

			// Remove HTML Development formatting
			// Replace line breaks with space
			// because browsers inserts space
			string result = source.Replace("\r", " ");
			// Replace line breaks with space
			// because browsers inserts space
			result = result.Replace("\n", " ");
			// Remove step-formatting
			result = result.Replace("\t", string.Empty);
			// Remove repeating spaces because browsers ignore them
			result = Regex.Replace(result, @"( )+", " ");

			// Remove the header (prepare first by clearing attributes)
			result = Regex.Replace(result, @"<( )*head([^>])*>", "<head>", RegexOptions.IgnoreCase);
			result = Regex.Replace(result, @"(<( )*(/)( )*head( )*>)", "</head>", RegexOptions.IgnoreCase);
			result = Regex.Replace(result, "(<head>).*(</head>)", string.Empty, RegexOptions.IgnoreCase);

			// remove all scripts (prepare first by clearing attributes)
			result = Regex.Replace(result, @"<( )*script([^>])*>", "<script>", RegexOptions.IgnoreCase);
			result = Regex.Replace(result, @"(<( )*(/)( )*script( )*>)", "</script>", RegexOptions.IgnoreCase);
			//result = Regex.Replace(result, @"(<script>)([^(<script>\.</script>)])*(</script>)",string.Empty, RegexOptions.IgnoreCase);
			result = Regex.Replace(result, @"(<script>).*(</script>)", string.Empty, RegexOptions.IgnoreCase);

			// remove all styles (prepare first by clearing attributes)
			result = Regex.Replace(result, @"<( )*style([^>])*>", "<style>", RegexOptions.IgnoreCase);
			result = Regex.Replace(result, @"(<( )*(/)( )*style( )*>)", "</style>", RegexOptions.IgnoreCase);
			result = Regex.Replace(result, "(<style>).*(</style>)", string.Empty, RegexOptions.IgnoreCase);

			// insert tabs in spaces of <td> tags
			result = Regex.Replace(result, @"<( )*td([^>])*>", "\t", RegexOptions.IgnoreCase);

			// insert line breaks in places of <BR> and <LI> tags
			result = Regex.Replace(result, @"<( )*br( )*>", "\r\n", RegexOptions.IgnoreCase);
			result = Regex.Replace(result, @"<( )*li( )*>", "\r\n", RegexOptions.IgnoreCase);

			// insert line paragraphs (double line breaks) in place
			// if <P>, <DIV> and <TR> tags
			result = Regex.Replace(result, @"<( )*div([^>])*>", "\r\r", RegexOptions.IgnoreCase);
			result = Regex.Replace(result, @"<( )*tr([^>])*>", "\r\r", RegexOptions.IgnoreCase);
			result = Regex.Replace(result, @"<( )*p([^>])*>", "\r\r", RegexOptions.IgnoreCase);

			// Remove remaining tags like <a>, links, images,
			// comments etc - anything that's enclosed inside < >
			result = Regex.Replace(result, @"<[^>]*>", string.Empty, RegexOptions.IgnoreCase);

			// replace special characters:
			result = Regex.Replace(result, @" ", " ", RegexOptions.IgnoreCase);

			result = Regex.Replace(result, @"&bull;", " * ", RegexOptions.IgnoreCase);
			result = Regex.Replace(result, @"&lsaquo;", "<", RegexOptions.IgnoreCase);
			result = Regex.Replace(result, @"&rsaquo;", ">", RegexOptions.IgnoreCase);
			result = Regex.Replace(result, @"&trade;", "(tm)", RegexOptions.IgnoreCase);
			result = Regex.Replace(result, @"&frasl;", "/", RegexOptions.IgnoreCase);
			result = Regex.Replace(result, @"&quot;", "\"", RegexOptions.IgnoreCase);
			result = Regex.Replace(result, @"&apos;", "'", RegexOptions.IgnoreCase);
			result = Regex.Replace(result, @"&lt;", "<", RegexOptions.IgnoreCase);
			result = Regex.Replace(result, @"&gt;", ">", RegexOptions.IgnoreCase);
			result = Regex.Replace(result, @"&copy;", "(c)", RegexOptions.IgnoreCase);
			result = Regex.Replace(result, @"&reg;", "(r)", RegexOptions.IgnoreCase);
			
			result = Regex.Replace(result, @"&#32;", " ", RegexOptions.IgnoreCase);
			result = Regex.Replace(result, @"&#33;", "!", RegexOptions.IgnoreCase);
			result = Regex.Replace(result, @"&#34;", "\"", RegexOptions.IgnoreCase);
			result = Regex.Replace(result, @"&#35;", "#", RegexOptions.IgnoreCase);
			result = Regex.Replace(result, @"&#36;", "$", RegexOptions.IgnoreCase);
			result = Regex.Replace(result, @"&#37;", "%", RegexOptions.IgnoreCase);
			result = Regex.Replace(result, @"&#38;", "&", RegexOptions.IgnoreCase);
			result = Regex.Replace(result, @"&#39;", "'", RegexOptions.IgnoreCase);
			result = Regex.Replace(result, @"&#40;", "(", RegexOptions.IgnoreCase);
			result = Regex.Replace(result, @"&#41;", ")", RegexOptions.IgnoreCase);
			result = Regex.Replace(result, @"&#42;", "*", RegexOptions.IgnoreCase);
			result = Regex.Replace(result, @"&#43;", "+", RegexOptions.IgnoreCase);
			result = Regex.Replace(result, @"&#44;", ",", RegexOptions.IgnoreCase);
			result = Regex.Replace(result, @"&#45;", "-", RegexOptions.IgnoreCase);
			result = Regex.Replace(result, @"&#46;", ".", RegexOptions.IgnoreCase);
			result = Regex.Replace(result, @"&#47;", "/", RegexOptions.IgnoreCase);

			result = Regex.Replace(result, @"&amp;", "&", RegexOptions.IgnoreCase);
			// Remove all others. More can be added, see
			// http://hotwired.lycos.com/webmonkey/reference/special_characters/
			result = Regex.Replace(result, @"&(.{2,6});", string.Empty, RegexOptions.IgnoreCase);

			// for testing
			//Regex.Replace(result,
			//       this.txtRegex.Text,string.Empty,
			//       RegexOptions.IgnoreCase);

			// make line breaking consistent
			//result = result.Replace("\n", "\r");

			// Remove extra line breaks and tabs:
			// replace over 2 breaks with 2 and over 4 tabs with 4.
			// Prepare first to remove any whitespaces in between
			// the escaped characters and remove redundant tabs in between line breaks
			result = Regex.Replace(result, "(\r)\\s+(\r)", "\r\r", RegexOptions.IgnoreCase);
			result = Regex.Replace(result, "(\t)( )+(\t)", "\t\t", RegexOptions.IgnoreCase);
			result = Regex.Replace(result, "(\t)( )+(\r)", "\t\r", RegexOptions.IgnoreCase);
			result = Regex.Replace(result, "(\r)( )+(\t)", "\r\t", RegexOptions.IgnoreCase);
			//// Remove redundant tabs
			//result = Regex.Replace(result, "(\r)(\t)+(\r)", "\r\r", RegexOptions.IgnoreCase);
			// Remove multiple tabs following a line break with just one tab
			result = Regex.Replace(result, "(\r)(\t)+", "\r\t", RegexOptions.IgnoreCase);

			result = Regex.Replace(result, "(\r)(\r)(\r)+", "\r\r", RegexOptions.IgnoreCase);
			result = Regex.Replace(result, "(\t)(\t)(\t)+", "\t\t", RegexOptions.IgnoreCase);

			// we need to change the \r to \r\n. we have to call this twice because the first time we only replace the first \r in \r\r
			result = Regex.Replace(result, "\r(?<Value>[^\n])", m => { return "\r\n" + m.Groups["Value"].Value; }, RegexOptions.IgnoreCase);
			result = Regex.Replace(result, "\r(?<Value>[^\n])", m => { return "\r\n" + m.Groups["Value"].Value; }, RegexOptions.IgnoreCase);


			// Initial replacement target string for line breaks
			//string breaks = "\r\r\r";
			//// Initial replacement target string for tabs
			//string tabs = "\t\t\t\t\t";
			//for (int index = 0; index < result.Length; index++)
			//{
			//    result = result.Replace(breaks, "\r\r");
			//    result = result.Replace(tabs, "\t\t\t\t");
			//    breaks = breaks + "\r";
			//    tabs = tabs + "\t";
			//}

			// That's it.
			return result.Trim();
		}

		/// <summary>
		/// return the number of times the character chr appears in the string
		/// </summary>
		/// <param name="str"></param>
		/// <param name="chr"></param>
		/// <returns></returns>
		public static int Count(this String str, char chr)
		{
			return str.Count(c => { return c == chr; });
		}

		/// <summary>
		/// Return the number of times the string str appears in the string
		/// </summary>
		/// <param name="input"></param>
		/// <param name="str"></param>
		/// <returns></returns>
		public static int Count(this String input, string str)
		{
			return Regex.Matches(input, str).Count;
		}

		public static string Capitalize(this string source)
		{
			if (String.IsNullOrEmpty(source))
				return source;

			if (source.Length == 1)
				return source.ToUpper();

			return String.Concat(source[0].ToString().ToUpper(), source.Substring(1).ToLower());
		}

		public static string CamelSplit(this string source)
		{
			if (String.IsNullOrEmpty(source))
				return String.Empty;

			var sb = new StringBuilder();

			foreach (var c in source)
			{
				if (Char.IsUpper(c))
					sb.Append(" ");

				sb.Append(c);
			}

			return sb.ToString().Trim();
		}

		///// <summary>
		///// format a string using a specified object.
		///// example call: "{CurrentTime} - {ProcessName}".FormatWith(new { CurrentTime = DateTime.Now, ProcessName = p.ProcessName });
		///// see: http://james.newtonking.com/archive/2008/03/29/formatwith-2-0-string-formatting-with-named-variables.aspx
		///// </summary>
		///// <param name="format"></param>
		///// <param name="source"></param>
		///// <returns></returns>
		//public static string FormatWith(this string format, object source)
		//{
		//    return FormatWith(format, null, source);
		//}

		//public static string FormatWith(this string format, IFormatProvider provider, object source)
		//{
		//    if (format == null)
		//        throw new ArgumentNullException("format");

		//    Regex r = new Regex(@"(?<start>\{)+(?<property>[\w\.\[\]]+)(?<format>:[^}]+)?(?<end>\})+",
		//      RegexOptions.Compiled | RegexOptions.CultureInvariant | RegexOptions.IgnoreCase);

		//    List<object> values = new List<object>();
		//    string rewrittenFormat = r.Replace(format, delegate(Match m)
		//    {
		//        Group startGroup = m.Groups["start"];
		//        Group propertyGroup = m.Groups["property"];
		//        Group formatGroup = m.Groups["format"];
		//        Group endGroup = m.Groups["end"];

		//        values.Add((propertyGroup.Value == "0")
		//          ? source
		//          : System.Web.UI.DataBinder.Eval(source, propertyGroup.Value));

		//        return new string('{', startGroup.Captures.Count) + (values.Count - 1) + formatGroup.Value
		//          + new string('}', endGroup.Captures.Count);
		//    });

		//    return string.Format(provider, rewrittenFormat, values.ToArray());
		//}

		/// <summary>
		/// An advanced version of string.Format.  If you pass a primitive object (string, int, etc), it acts like the regular string.Format.  If you pass an anonmymous type, you can name the paramters by property name.
		/// </summary>
		/// <param name="formatString"></param>
		/// <param name="arg"></param>
		/// <returns></returns>
		/// <example>
		/// "The {Name} family has {Children} children".Format(new { Children = 4, Name = "Smith" })
		/// 
		/// results in 
		/// "This Smith family has 4 children
		/// </example>
		public static string Format(this string formatString, object arg, IFormatProvider format = null)
		{
			if (arg == null)
				return formatString;
			if (string.IsNullOrEmpty(formatString))
				return formatString;

			var type = arg.GetType();
			if (Type.GetTypeCode(type) != TypeCode.Object || type.IsPrimitive)
				return string.Format(format, formatString, arg);

			var properties = System.ComponentModel.TypeDescriptor.GetProperties(arg);
			return formatString.Format((property) =>
			{
				try {
					var value = properties[property].GetValue(arg);
					return Convert.ToString(value, format);
				}
				catch (Exception ex) {
					throw new ArgumentException("Missing property " + property);
				}
			});
		}


		public static string Format(this string formatString, Func<string, string> formatFragmentHandler)
		{
			if (string.IsNullOrEmpty(formatString))
				return formatString;
			Fragment[] fragments = GetParsedFragments(formatString);
			if (fragments == null || fragments.Length == 0)
				return formatString;

			return string.Join(string.Empty, fragments.Select(fragment =>
			{
				if (fragment.Type == FragmentType.Literal)
					return fragment.Value;
				else
					return formatFragmentHandler(fragment.Value);
			}).ToArray());
		}

		public static string RemoveWhiteSpaces(this string text)
		{
			if (string.IsNullOrEmpty(text))
				return text;
			return Regex.Replace(text, @"\s", "");
		}


		private static Fragment[] GetParsedFragments(string formatString)
		{
			Fragment[] fragments;
			if (parsedStrings.TryGetValue(formatString, out fragments))
			{
				return fragments;
			}
			lock (parsedStringsLock)
			{
				if (!parsedStrings.TryGetValue(formatString, out fragments))
				{
					fragments = Parse(formatString);
					parsedStrings.Add(formatString, fragments);
				}
			}
			return fragments;
		}

		private static Object parsedStringsLock = new Object();
		private static Dictionary<string, Fragment[]> parsedStrings = new Dictionary<string, Fragment[]>(StringComparer.Ordinal);

		const char OpeningDelimiter = '{';
		const char ClosingDelimiter = '}';

		/// <summary>
		/// Parses the given format string into a list of fragments.
		/// </summary>
		/// <param name="format"></param>
		/// <returns></returns>
		static Fragment[] Parse(string format)
		{
			int lastCharIndex = format.Length - 1;
			int currFragEndIndex;
			Fragment currFrag = ParseFragment(format, 0, out currFragEndIndex);

			if (currFragEndIndex == lastCharIndex)
			{
				return new Fragment[] { currFrag };
			}

			List<Fragment> fragments = new List<Fragment>();
			while (true)
			{
				fragments.Add(currFrag);
				if (currFragEndIndex == lastCharIndex)
				{
					break;
				}
				currFrag = ParseFragment(format, currFragEndIndex + 1, out currFragEndIndex);
			}
			return fragments.ToArray();

		}

		/// <summary>
		/// Finds the next delimiter from the starting index.
		/// </summary>
		static Fragment ParseFragment(string format, int startIndex, out int fragmentEndIndex)
		{
			bool foundEscapedDelimiter = false;
			FragmentType type = FragmentType.Literal;

			int numChars = format.Length;
			for (int i = startIndex; i < numChars; i++)
			{
				char currChar = format[i];
				bool isOpenBrace = currChar == OpeningDelimiter;
				bool isCloseBrace = isOpenBrace ? false : currChar == ClosingDelimiter;

				if (!isOpenBrace && !isCloseBrace)
				{
					continue;
				}
				else if (i < (numChars - 1) && format[i + 1] == currChar)
				{//{{ or }}
					i++;
					foundEscapedDelimiter = true;
				}
				else if (isOpenBrace)
				{
					if (i == startIndex)
					{
						type = FragmentType.FormatItem;
					}
					else
					{

						if (type == FragmentType.FormatItem)
							throw new FormatException("Two consequtive unescaped { format item openers were found.  Either close the first or escape any literals with another {.");

						//curr character is the opening of a new format item.  so we close this literal out
						string literal = format.Substring(startIndex, i - startIndex);
						if (foundEscapedDelimiter)
							literal = ReplaceEscapes(literal);

						fragmentEndIndex = i - 1;
						return new Fragment(FragmentType.Literal, literal);
					}
				}
				else
				{//close bracket
					if (i == startIndex || type == FragmentType.Literal)
						throw new FormatException("A } closing brace existed without an opening { brace.");

					string formatItem = format.Substring(startIndex + 1, i - startIndex - 1);
					if (foundEscapedDelimiter)
						formatItem = ReplaceEscapes(formatItem);//a format item with a { or } in its name is crazy but it could be done
					fragmentEndIndex = i;
					return new Fragment(FragmentType.FormatItem, formatItem);
				}
			}

			if (type == FragmentType.FormatItem)
				throw new FormatException("A format item was opened with { but was never closed.");

			fragmentEndIndex = numChars - 1;
			string literalValue = format.Substring(startIndex);
			if (foundEscapedDelimiter)
				literalValue = ReplaceEscapes(literalValue);

			return new Fragment(FragmentType.Literal, literalValue);

		}

		/// <summary>
		/// Replaces escaped brackets, turning '{{' and '}}' into '{' and '}', respectively.
		/// </summary>
		/// <param name="value"></param>
		/// <returns></returns>
		static string ReplaceEscapes(string value)
		{
			return value.Replace("{{", "{").Replace("}}", "}");
		}

		#region check content
		public static bool IsInt(this string text)
		{
			int foo;
			return int.TryParse(text, out foo);
		}

		public static bool IsFloat(this string text)
		{
			float foo;
			return float.TryParse(text, out foo);
		}
		#endregion

		#region string encoding
		public static byte[] ConvertToUtf8(this string source)
		{
			return new UTF8Encoding(true).GetBytes(source);
		}
		#endregion

		#region Trimming functions
		/// <summary>
		/// trim away the value from the beginning of the string (if it exists)
		/// </summary>
		/// <param name="source"></param>
		/// <param name="value"></param>
		/// <returns></returns>
		public static string TrimStart(this string source, string value)
		{
			if (source.StartsWith(value))
				source = source.Substring(value.Length);
			return source;
		}

		/// <summary>
		/// trim away the value from the end of the string (if it exists)
		/// </summary>
		/// <param name="source"></param>
		/// <param name="value"></param>
		/// <returns></returns>
		public static string TrimEnd(this string source, string value)
		{
			if (source.EndsWith(value))
				source = source.Substring(0, source.Length - value.Length);
			return source;
		}

		/// <summary>
		/// trim a substring from the beginning or the end of the string
		/// </summary>
		/// <param name="source"></param>
		/// <param name="value"></param>
		/// <returns></returns>
		public static string Trim(this string source, string value)	{
			source = source.TrimStart(value);
			source = source.TrimEnd(value);
			return source;
		}

		/// <summary>
		/// trim away from the string any of the strings in the values collection (the order of the string is important!! 
		/// strings are removed in consecutive order)
		/// </summary>
		/// <param name="source"></param>
		/// <param name="values"></param>
		/// <returns></returns>
		public static string Trim(this string source, IEnumerable<string> values)
		{
			foreach (string value in values)
				source = source.Trim(value);
			return source;
		}
		#endregion

		#region hashing functions
		[SuppressMessage("Microsoft.Performance", "CA1811:AvoidUncalledPrivateCode")]
		public static string GetMD5HashString(this string value)
		{
			using (MD5 md5 = MD5.Create())
			{
				byte[] signatureHash = md5.ComputeHash(Encoding.UTF8.GetBytes(value));
				string signature = signatureHash.Aggregate(
					new StringBuilder(),
					(sb, b) => sb.Append(b.ToString("x2", CultureInfo.InvariantCulture))).ToString();
				return signature;
			}
		}

		public static string CalculateMD5Hash(this string content)
		{
			return Text.CalculateMD5Hash(content);
		}

		public static string CalculateSHA1(this string content)
		{
			return Text.CalculateSHA1(content);
		}

		public static string ComputeSHA512(this string content)
		{
			return Text.ComputeSHA512(content);
		}
		#endregion

		#region encryption
		/// <summary>
		/// encrypt the string using the given password
		/// </summary>
		/// <param name="clearText"></param>
		/// <param name="password"></param>
		/// <returns></returns>
		public static string Encrypt(this string clearText, string password)
		{
			if (string.IsNullOrEmpty(clearText))
				return clearText;
			return Text.Encrypt(clearText, password);
		}

		/// <summary>
		/// decrypt the string using the given password
		/// </summary>
		/// <param name="cipherText"></param>
		/// <param name="password"></param>
		/// <returns></returns>
		public static string Decrypt(this string cipherText, string password)
		{
			if (string.IsNullOrEmpty(cipherText))
				return cipherText;
			return Text.Decrypt(cipherText, password);
		}
		#endregion

		private enum FragmentType
		{
			Literal,
			FormatItem
		}

		private class Fragment
		{

			public Fragment(FragmentType type, string value)
			{
				Type = type;
				Value = value;
			}

			public FragmentType Type
			{
				get;
				private set;
			}

			/// <summary>
			/// The literal value, or the name of the fragment, depending on fragment type.
			/// </summary>
			public string Value
			{
				get;
				private set;
			}
		}
	}
}
