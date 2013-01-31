using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Net;
using System.IO;
using System.Diagnostics;
using System.Text.RegularExpressions;

namespace GenLib.Web
{
	public class Utility
	{
		public static Dictionary<string, string> ParseQueryArguments(string url, string argumentDelimiters = "&", bool urlEncoded = true)
		{
			Dictionary<string, string> keyValDict = new Dictionary<string,string>();
			try 
			{	        
				if (url.Contains('?'))
						url = url.Substring(url.IndexOf('?') + 1);

				int num = (url != null) ? url.Length : 0;
				for (int i = 0; i < num; i++)
				{
					int startIndex = i;
					int borderIndex = -1;
					while (i < num)
					{
						char ch = url[i];
						if (ch == '=')
						{
							if (borderIndex < 0)
								borderIndex = i;
						}
						else if (argumentDelimiters.Contains(ch))
							break;
						i++;
					}
					string key = null;
					string value = null;
					if (borderIndex >= 0)
					{
						key = url.Substring(startIndex, borderIndex - startIndex);
						value = url.Substring(borderIndex + 1, (i - borderIndex) - 1);
					}
					else
						value = url.Substring(startIndex, i - startIndex);
				
					if (urlEncoded)
						keyValDict[System.Web.HttpUtility.UrlDecode(key)] = System.Web.HttpUtility.UrlDecode(value);
					else
					{
						keyValDict[key] = value;
					}
				}
			}
			catch (Exception ex)
			{
				Debug.WriteLine("Exception while parsing query: " + ex.Message);
			}
			
			return keyValDict;
		}

		public static bool SaveFileFromURL(string url, string destinationFileName, int timeoutInSeconds)
		{
			// Create a web request to the URL
			HttpWebRequest MyRequest = (HttpWebRequest)WebRequest.Create(url);
			MyRequest.Timeout = timeoutInSeconds * 1000;
			try
			{
				// Get the web response
				HttpWebResponse MyResponse = (HttpWebResponse)MyRequest.GetResponse();

				// Make sure the response is valid
				if (HttpStatusCode.OK == MyResponse.StatusCode)
				{
					// Open the response stream
					using (Stream MyResponseStream = MyResponse.GetResponseStream())
					{
						// Open the destination file
						using (FileStream MyFileStream = new FileStream(destinationFileName, FileMode.OpenOrCreate, FileAccess.Write))
						{
							// Create a 4K buffer to chunk the file
							byte[] MyBuffer = new byte[4096];
							int BytesRead;
							// Read the chunk of the web response into the buffer
							while (0 < (BytesRead = MyResponseStream.Read(MyBuffer, 0, MyBuffer.Length)))
							{
								// Write the chunk from the buffer to the file
								MyFileStream.Write(MyBuffer, 0, BytesRead);
							}
						}
					}
				}
			}
			catch (Exception ex)
			{
				Trace.WriteLine("SaveFileFromURL - Error saving file from URL:" + ex.Message);
			}
			return true;
		}

		/// <summary>
		/// return <a href=link>text</a>
		/// </summary>
		/// <param name="link"></param>
		/// <param name="text"></param>
		/// <returns></returns>
		public static string CreateWebLink(string link, string text)
		{
			return String.Format("<a href=\"{0}\">{1}</a>", link, text);
		}

		// use the cookiecontainter to retrieve the content of the given url
		public static string GetWebPageContent(string url, CookieContainer cookieContainer = null)
		{
			HttpWebRequest request = (HttpWebRequest)WebRequest.Create(url);
			request.MaximumAutomaticRedirections = 4;
			request.MaximumResponseHeadersLength = 4;
			request.Method = "GET";
			request.Credentials = CredentialCache.DefaultCredentials;
			request.CookieContainer = cookieContainer;
			request.Headers.Add(HttpRequestHeader.AcceptEncoding, "gzip,deflate");

			HttpWebResponse response = (HttpWebResponse)request.GetResponse();

			Stream responseStream = response.GetResponseStream();
			if (response.ContentEncoding.ToLower().Contains("gzip"))
				responseStream = new System.IO.Compression.GZipStream(responseStream, System.IO.Compression.CompressionMode.Decompress);
			else if (response.ContentEncoding.ToLower().Contains("deflate"))
				responseStream = new System.IO.Compression.DeflateStream(responseStream, System.IO.Compression.CompressionMode.Decompress);

			StreamReader readStream = new StreamReader(responseStream, Encoding.UTF8);

			string html = readStream.ReadToEnd();
			
			response.Close();
			responseStream.Close();
			return html;
		}
	}
}
