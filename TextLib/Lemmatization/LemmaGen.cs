using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using LemmaSharp;

namespace TextLib.Lemmatization
{
	public class LemmaGen
	{
		private Dictionary<string, string> _wordToLemma = new Dictionary<string, string>();
		private ILemmatizer _lemmatizer = null;

		public LemmaGen()
		{
			_lemmatizer = new LemmatizerPrebuiltCompact(LemmaSharp.LanguagePrebuilt.English);
		}

		public LemmaGen(LemmaSharp.LanguagePrebuilt language)
		{
			_lemmatizer = new LemmatizerPrebuiltCompact(language);
		}
		
		public string GetLemma(string word)
		{
			//word = Tokenization.NormalizeText(word);
			if (!_wordToLemma.ContainsKey(word))
				_wordToLemma[word] = _lemmatizer.Lemmatize(word);
			//_wordToLemma[word] = lemmatizer.Lemmatize(word.ToLower());
			return _wordToLemma[word];
		}

		public void SetCustomLemmas(List<string> customLemmas)
		{
			foreach (var word in customLemmas)
				_wordToLemma[word] = word;
		}
	}
}
