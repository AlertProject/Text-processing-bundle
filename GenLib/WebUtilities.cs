using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;

using System.Globalization;
using System.IO;
using System.Net;
using System.Text.RegularExpressions;
using System.Threading;
using System.Web;
using System.Diagnostics;
using System.ComponentModel;
using System.Diagnostics.CodeAnalysis;
using GenLib.Misc;

namespace GenLib
{
	class WebUtilities
	{
		public enum SearchType
		{
			GOOGLE,
			LINKEDIN,
			FACEBOOK,
			TWITTER
		}

		public static string GetWebPageAsString(string url)
		{
			string userAgent = "Mozilla/4.0 (compatible; MSIE 6.0; Windows NT 5.1; )";
			return GetWebPageAsString(url, userAgent);
		}

		public static string GetWebPageAsString(string url, string userAgent)
		{
			string str = string.Empty;
			HttpWebResponse response = null;
			try
			{
				HttpWebRequest request = (HttpWebRequest)WebRequest.Create(url);
				request.Timeout = 0x2710;
				request.UserAgent = userAgent;
				response = (HttpWebResponse)request.GetResponse();
				str = new StreamReader(response.GetResponseStream(), Encoding.ASCII).ReadToEnd();
			}
			catch (WebException exception)
			{
				if ((exception.Response != null) && (exception.Response.Headers != null))
				{
					str = exception.Response.Headers.Get("WWW-Authenticate");
					if (str == null)
					{
						str = string.Empty;
					}
				}
			}
			catch (HttpException)
			{
			}
			catch (IOException)
			{
			}
			return str;
		}

		public static void LaunchSearch(string searchTerm, SearchType searchType)
		{
			if (!string.IsNullOrEmpty(searchTerm))
			{
				searchTerm = searchTerm.Replace(' ', '+');
				string str = string.Empty;
				switch (searchType)
				{
					case SearchType.GOOGLE:
						str = "http://www.google.com/search?q={0}";
						break;

					case SearchType.LINKEDIN:
						str = "http://www.linkedin.com/search?pplSearchOrigin=GLHD&keywords={0}&search=";
						break;

					case SearchType.FACEBOOK:
						str = "http://www.facebook.com/srch.php?nm={0}";
						break;

					case SearchType.TWITTER:
						str = "http://twitter.com/search/users?q={0}&category=people&source=users";
						break;
				}
				if (!string.IsNullOrEmpty(str))
				{
					LaunchUrl(Uri.EscapeUriString(string.Format(CultureInfo.InvariantCulture, str, new object[] { searchTerm })));
				}
			}
		}

		public static void LaunchTwitterNameSearch(string name)
		{
			LaunchSearch(name, SearchType.TWITTER);
		}

		public static void LaunchTwitterProfile(string twitterId)
		{
			if (!string.IsNullOrEmpty(twitterId))
			{
				LaunchUrl(Uri.EscapeUriString(string.Format(CultureInfo.InvariantCulture, "http://twitter.com/{0}", new object[] { twitterId })));
			}
		}

		public static void LaunchUrl(string url)
		{
			if (!string.IsNullOrEmpty(url))
			{
				try
				{
					Process.Start(url);
				}
				catch (Win32Exception exception)
				{
					//MessageBox.Show(string.Format(CultureInfo.CurrentCulture, "Cannot navigate to {0}.\r\n{1}", new object[] { url, exception.Message }), Resources.ApplicationName, MessageBoxButtons.OK, MessageBoxIcon.Hand);
					Debug.WriteLine(string.Format(CultureInfo.CurrentCulture, "Cannot navigate to {0}.\r\n{1}", url, exception.Message));
				}
				catch (Exception exception2)
				{
					//ExceptionHandler.HandleAndReport(exception2);
					Debug.WriteLine(string.Format(CultureInfo.CurrentCulture, "Cannot navigate to {0}.\r\n{1}", url, exception2.Message));
				}
			}
		}

		public static bool TrySaveUrlToFile(string url, string filepath)
		{
			bool flag = false;
			if (Uri.IsWellFormedUriString(url, UriKind.Absolute))
			{
				try
				{
					HttpWebRequest request = (HttpWebRequest)WebRequest.Create(url);
					HttpWebResponse response = (HttpWebResponse)request.GetResponse();
					StreamReader reader = new StreamReader(response.GetResponseStream(), Encoding.ASCII);
					StreamWriter writer = new StreamWriter(filepath, false);
					string str = reader.ReadToEnd();
					writer.Write(str);
					writer.Close();
					flag = true;
				}
				catch (HttpException)
				{
				}
				catch (WebException)
				{
				}
				catch (IOException)
				{
				}
			}
			return flag;
		}

		//[SuppressMessage("Microsoft.Performance", "CA1811:AvoidUncalledPrivateCode")]
		public static string UrlDecode(string url)
		{
			if (url == null)
			{
				return null;
			}

			var decoder = new _UrlDecoder(url.Length, Encoding.UTF8);
			int length = url.Length;
			for (int i = 0; i < length; ++i)
			{
				char ch = url[i];

				if (ch == '+')
				{
					decoder.AddByte((byte)' ');
					continue;
				}

				if (ch == '%' && i < length - 2)
				{
					// decode %uXXXX into a Unicode character.
					if (url[i + 1] == 'u' && i < length - 5)
					{
						int a = _HexToInt(url[i + 2]);
						int b = _HexToInt(url[i + 3]);
						int c = _HexToInt(url[i + 4]);
						int d = _HexToInt(url[i + 5]);
						if (a >= 0 && b >= 0 && c >= 0 && d >= 0)
						{
							decoder.AddChar((char)((a << 12) | (b << 8) | (c << 4) | d));
							i += 5;

							continue;
						}
					}
					else
					{
						// decode %XX into a Unicode character.
						int a = _HexToInt(url[i + 1]);
						int b = _HexToInt(url[i + 2]);

						if (a >= 0 && b >= 0)
						{
							decoder.AddByte((byte)((a << 4) | b));
							i += 2;

							continue;
						}
					}
				}

				// Add any 7bit character as a byte.
				if ((ch & 0xFF80) == 0)
				{
					decoder.AddByte((byte)ch);
				}
				else
				{
					decoder.AddChar(ch);
				}
			}

			return decoder.GetString();
		}

		/// <summary>
		/// Encodes a URL string.  Duplicated functionality from System.Web.HttpUtility.UrlEncode.
		/// </summary>
		/// <param name="url"></param>
		/// <returns></returns>
		/// <remarks>
		/// Duplicated from System.Web.HttpUtility because System.Web isn't part of the client profile.
		/// URL Encoding replaces ' ' with '+' and unsafe ASCII characters with '%XX'.
		/// Safe characters are defined in RFC2396 (http://www.ietf.org/rfc/rfc2396.txt).
		/// They are the 7-bit ASCII alphanumerics and the mark characters "-_.!~*'()".
		/// This implementation does not treat '~' as a safe character to be consistent with the System.Web version.
		/// </remarks>
		//[SuppressMessage("Microsoft.Performance", "CA1811:AvoidUncalledPrivateCode")]
		public static string UrlEncode(string url)
		{
			if (url == null)
			{
				return null;
			}

			byte[] bytes = Encoding.UTF8.GetBytes(url);

			bool needsEncoding = false;
			int unsafeCharCount = 0;
			foreach (byte b in bytes)
			{
				if (b == ' ')
				{
					needsEncoding = true;
				}
				else if (!_UrlEncodeIsSafe(b))
				{
					++unsafeCharCount;
					needsEncoding = true;
				}
			}

			if (needsEncoding)
			{
				var buffer = new byte[bytes.Length + (unsafeCharCount * 2)];
				int writeIndex = 0;
				foreach (byte b in bytes)
				{
					if (_UrlEncodeIsSafe(b))
					{
						buffer[writeIndex++] = b;
					}
					else if (b == ' ')
					{
						buffer[writeIndex++] = (byte)'+';
					}
					else
					{
						buffer[writeIndex++] = (byte)'%';
						buffer[writeIndex++] = _IntToHex((b >> 4) & 0xF);
						buffer[writeIndex++] = _IntToHex(b & 0xF);
					}
				}
				bytes = buffer;
				Assert.AreEqual(buffer.Length, writeIndex);
			}

			return Encoding.ASCII.GetString(bytes);
		}

		[SuppressMessage("Microsoft.Performance", "CA1811:AvoidUncalledPrivateCode")]
		private static bool _IsAsciiAlphaNumeric(byte b)
		{
			return (b >= 'a' && b <= 'z')
				|| (b >= 'A' && b <= 'Z')
				|| (b >= '0' && b <= '9');
		}

		[SuppressMessage("Microsoft.Performance", "CA1811:AvoidUncalledPrivateCode")]
		private static byte _IntToHex(int n)
		{
			Assert.BoundedInteger(0, n, 16);
			if (n <= 9)
			{
				return (byte)(n + '0');
			}
			return (byte)(n - 10 + 'A');
		}

		[SuppressMessage("Microsoft.Performance", "CA1811:AvoidUncalledPrivateCode")]
		private static int _HexToInt(char h)
		{
			if (h >= '0' && h <= '9')
			{
				return h - '0';
			}

			if (h >= 'a' && h <= 'f')
			{
				return h - 'a' + 10;
			}

			if (h >= 'A' && h <= 'F')
			{
				return h - 'A' + 10;
			}

			Assert.Fail("Invalid hex character " + h);
			return -1;
		}
		
		// HttpUtility's UrlEncode is slightly different from the RFC.
		// RFC2396 describes unreserved characters as alphanumeric or
		// the list "-" | "_" | "." | "!" | "~" | "*" | "'" | "(" | ")"
		// The System.Web version unnecessarily escapes '~', which should be okay...
		// Keeping that same pattern here just to be consistent.
		//[SuppressMessage("Microsoft.Performance", "CA1811:AvoidUncalledPrivateCode")]
		private static bool _UrlEncodeIsSafe(byte b)
		{
			if (_IsAsciiAlphaNumeric(b))
			{
				return true;
			}

			switch ((char)b)
			{
				case '-':
				case '_':
				case '.':
				case '!':
				//case '~':
				case '*':
				case '\'':
				case '(':
				case ')':
					return true;
			}

			return false;
		}

		private class _UrlDecoder
		{
			private readonly Encoding _encoding;
			private readonly char[] _charBuffer;
			private readonly byte[] _byteBuffer;
			private int _byteCount;
			private int _charCount;

			[SuppressMessage("Microsoft.Performance", "CA1811:AvoidUncalledPrivateCode")]
			public _UrlDecoder(int size, Encoding encoding)
			{
				_encoding = encoding;
				_charBuffer = new char[size];
				_byteBuffer = new byte[size];
			}

			[SuppressMessage("Microsoft.Performance", "CA1811:AvoidUncalledPrivateCode")]
			public void AddByte(byte b)
			{
				_byteBuffer[_byteCount++] = b;
			}

			[SuppressMessage("Microsoft.Performance", "CA1811:AvoidUncalledPrivateCode")]
			public void AddChar(char ch)
			{
				_FlushBytes();
				_charBuffer[_charCount++] = ch;
			}

			[SuppressMessage("Microsoft.Performance", "CA1811:AvoidUncalledPrivateCode")]
			private void _FlushBytes()
			{
				if (_byteCount > 0)
				{
					_charCount += _encoding.GetChars(_byteBuffer, 0, _byteCount, _charBuffer, _charCount);
					_byteCount = 0;
				}
			}

			[SuppressMessage("Microsoft.Performance", "CA1811:AvoidUncalledPrivateCode")]
			public string GetString()
			{
				_FlushBytes();
				if (_charCount > 0)
				{
					return new string(_charBuffer, 0, _charCount);
				}
				return "";
			}
		}


	}
}
