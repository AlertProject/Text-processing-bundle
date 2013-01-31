using System;
using System.Collections.Generic;
using System.Linq;
using System.Web;
using System.Diagnostics;
using System.Text.RegularExpressions;

namespace TextLib.TextMining
{
	[DebuggerDisplay("Token: {Text}, Position= {Position}")]
	public class Token
	{
		public string Text { get; private set; }
		public int Position { get; private set; }
		
		public Token(string text, int position)
		{
			Text = text;
			Position = position;
		}

	}

	public class Tokenization
	{
		private static string _wordSplitString = " ,.)([];:";
		private static char[] _wordSplitChars = _wordSplitString.ToCharArray();

		private static string _termSplitString = ",.)([];:";
		private static char[] _termSplitChars = _termSplitString.ToCharArray();

		//private static Regex _tokenizerRegex = new Regex(@"(?<token>(((?#web links)(http://|https://|ftp://|www.)[^ ""]+)|(?#ordinary tokens)[^ ,.)(\[\];:""?!]+))([ ,.\[\];:""?!]|$)+");
		private static Regex _tokenizerRegex = new Regex(@"(?<token>(((?#web links)(http://|https://|ftp://|www.)[^ ""]+)|(?#ordinary tokens).+?))([^'\w]|$)+", RegexOptions.IgnoreCase | RegexOptions.Multiline);

		/// <summary>
		/// return a list of tokens, where for each token we return the term + the position of the term in the text
		/// </summary>
		/// <param name="text">input text to be tokenized</param>
		/// <returns>a list of tokens and their positions</returns>
		public static List<Token> GetTokens(string text)
		{
			List<Token> tokens = new List<Token>();
			foreach (Match token in _tokenizerRegex.Matches(text))
			{
				tokens.Add(new Token(token.Groups["token"].Value, token.Groups["token"].Index));
			}

			//int index = 0;
			//int lastWordIndex = 0;
			//while (index < text.Length)
			//{
			//    // we have come to the end of a word. add a new token with it
			//    // todo: this will not work well in finding source code files, eg. maildata.cpp - we'll get maildata. 
			//    // maybe simply use a dictionary based approach to detecting these cases. if after . we have cpp, c, ... then don't break the word
			//    if (_wordSplitString.Contains(text[index]))
			//    {
			//        if (index > lastWordIndex)
			//            tokens.Add(new Token(text.Substring(lastWordIndex, index - lastWordIndex), lastWordIndex));
			//        index++;
			//        lastWordIndex = index;			
			//    }
			//    else
			//        index++;
			//}
			//if (index > lastWordIndex)
			//    tokens.Add(new Token(text.Substring(lastWordIndex, index - lastWordIndex), lastWordIndex));
			return tokens;
		}

		public static string[] GetTextParts(string text)
		{
			return text.Split(_wordSplitChars, StringSplitOptions.RemoveEmptyEntries);
		}

		public static string JoinStringParts(string[] parts)
		{
			return String.Join(" ", parts);
		}

		public static string NormalizeText(string text)
		{
			return text.ToLower(System.Globalization.CultureInfo.InvariantCulture);
		}

		/// <summary>
		/// returns true if the given text doesn't contain any chars that suggest that the words in the text can't belong to the same term
		/// for example: "usb key" -> true. "usb. Key" -> false
		/// </summary>
		/// <param name="text"></param>
		/// <returns></returns>
		public static bool IsTermCandidate(string text)
		{
			foreach (char c in _termSplitChars)
				if (text.Contains(c)) return false;
			return true;
		}

		public static bool IsNumeric(string text)
		{
			try
			{
				double val;
				return double.TryParse(text, out val);
			}
			catch (Exception) { return false; }
		}
	}
}