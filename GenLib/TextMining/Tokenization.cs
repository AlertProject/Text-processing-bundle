using System;
using System.Collections.Generic;
using System.Linq;
#if! SILVERLIGHT
using System.Web;
#endif
using System.Diagnostics;


namespace GenLib.TextMining
{
	[DebuggerDisplay("Token: {_text}, Position= {_position}")]
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

		/// <summary>
		/// return a list of tokens, where for each token we return the term + the position of the term in the text
		/// </summary>
		/// <param name="text">input text to be tokenized</param>
		/// <returns>a list of tokens and their positions</returns>
		public static List<Token> GetTokens(string text)
		{
			List<Token> tokens = new List<Token>();
			int index = 0;
			int lastWordIndex = 0;
			while (index < text.Length)
			{
				// we have come to the end of a word. add a new token with it
				// todo: this will not work well in finding source code files, eg. maildata.cpp - we'll get maildata. 
				// maybe simply use a dictionary based approach to detecting these cases. if after . we have cpp, c, ... then don't break the word
				if (_wordSplitString.Contains(text[index]))
				{
					if (index > lastWordIndex)
					{
						tokens.Add(new Token(text.Substring(lastWordIndex, index - lastWordIndex), lastWordIndex));
					}
					index++;
					lastWordIndex = index;			
				}
				else
					index++;
			}
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
			return text.ToLower();
		}
	}
}