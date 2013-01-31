using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Text.RegularExpressions;
using System.Diagnostics;

using System.Globalization;
using System.IO;
using System.Xml;
using System.Security.Cryptography;

namespace GenLib.Text
{
	public static class Text
	{
		static public string EncodeXMLString(string value)
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

		static public string DecodeXMLString(string value)
		{
			if (string.IsNullOrEmpty(value)) return value;
			value = DecodeNonAsciiCharacters(value);
			value = value.Replace("&quot;", "\"");
			value = value.Replace("&apos;", "'");
			value = value.Replace("&lt;", "<");
			value = value.Replace("&gt;", ">");
			value = value.Replace("&amp;", "&");
			return value;
		}

		static public string StripHtml(string text)
		{
			if (string.IsNullOrEmpty(text))
				return text;
			return text.StripHtml();
		}

		/// <summary>
		/// remove everything between x00-x1f except 9,10 and 13 (\t\r\n)
		/// </summary>
		/// <param name="value"></param>
		/// <returns></returns>
		//static Regex RegExControlChar = new Regex(@"[\x00-\x08\x0B\x0C\x0E-\x1F\x7F]", RegexOptions.IgnoreCase);
		static public string RemoveControlCharacters(string value)
		{
			if (string.IsNullOrEmpty(value)) return value;
			StringBuilder sb = new StringBuilder();
			foreach (char c in value)
			{
				if (c > 31 && c != 127)
					sb.Append(c);
				else if (c == 9 || c == 10 || c == 13)
					sb.Append(c);
			}
			return sb.ToString();
		}

		static public string EncodeNonAsciiCharacters(string value)
		{
			if (string.IsNullOrEmpty(value)) return value;
			StringBuilder sb = new StringBuilder();
			foreach (char c in value)
			{
				if ((c > 127) || (c < 32 && c != 9 && c != 10 && c != 13))		// above 127 or below 32 has to be encoded. 9=TAB, 10=LF, 13=CR
				{
					// This character is too big for ASCII
					//string encodedValue = "\\u" + ((int)c).ToString("x4");
					//sb.Append("@_" + ((int)c).ToString("x4") + "_@");
					sb.Append("&#" + (int)c + ";");
				}
				else
				{
					sb.Append(c);
				}
			}
			return sb.ToString();
		}

		static public string DecodeNonAsciiCharacters(string value)
		{
			if (string.IsNullOrEmpty(value)) return value;
			return System.Text.RegularExpressions.Regex.Replace(
				//value, @"\\u(?<Value>[a-zA-Z0-9]{4})", m => { return ((char)int.Parse(m.Groups["Value"].Value, System.Globalization.NumberStyles.HexNumber)).ToString(); });
				//value, @"@_(?<Value>[a-zA-Z0-9]{4})_@", m => { return ((char)int.Parse(m.Groups["Value"].Value, System.Globalization.NumberStyles.HexNumber)).ToString(); });
				value, @"&#(?<Value>[0-9]+);", m => { return ((char)int.Parse(m.Groups["Value"].Value)).ToString(); });
		}

#if! SILVERLIGHT
		// capitalize each word in the string
		public static string CapitalizeWords(string name)
		{
			if (string.IsNullOrEmpty(name)) return name;
			return CultureInfo.CurrentCulture.TextInfo.ToTitleCase(name);
		}

		// replace unicode characters with their ascii approximations - replace ščć with scc and so on
		public static string ReplaceUnicodeCharsWithAscii(string text)
		{
			if (string.IsNullOrEmpty(text)) return text;
			string normalized = text.Normalize(NormalizationForm.FormKD);
			Encoding ascii = Encoding.GetEncoding("us-ascii", new EncoderReplacementFallback(string.Empty), new DecoderReplacementFallback(string.Empty));
			byte[] encodedBytes = new byte[ascii.GetByteCount(normalized)];
			int numberOfEncodedBytes = ascii.GetBytes(normalized, 0, normalized.Length, encodedBytes, 0);
			return ascii.GetString(encodedBytes);
		}
#endif 
				
		public static float GetStringPartsSimilarity(string s1, string s2)
		{
			if (string.IsNullOrEmpty(s1) || string.IsNullOrEmpty(s2)) return 0;
			string[] p1 = s1.ToLower().Split(new char[] { ' ', '.', ',' });
			string[] p2 = s2.ToLower().Split(new char[] { ' ', '.', ',' });
			var matches1 = from p in p1 where p2.Contains(p) select p;
			var matches2 = from p in p2 where p1.Contains(p) select p;
			int matches = matches1.Count() + matches2.Count();
			return (float) Math.Max(0.0, Math.Min(1.0, (matches / (float)(p1.Count()+p2.Count()))));

		}

		//public static int CountOccurencesOfChar(string text, char c)
		//{
		//    if (string.IsNullOrEmpty(text)) return 0;
		//    return text.Count(ch => ch == c);
		//    //int result = 0;
		//    //foreach (char curChar in instance)
		//    //{
		//    //    if (c == curChar)
		//    //        ++result;
		//    //}
		//    //return result;
		//}

		public static Int32 LevenshteinDistance(String a, String b)
		{
			if (string.IsNullOrEmpty(a))
			{
				if (!string.IsNullOrEmpty(b))
				{
					return b.Length;
				}
				return 0;
			}

			if (string.IsNullOrEmpty(b))
			{
				if (!string.IsNullOrEmpty(a))
				{
					return a.Length;
				}
				return 0;
			}

			Int32 cost;
			Int32[,] d = new int[a.Length + 1, b.Length + 1];
			Int32 min1;
			Int32 min2;
			Int32 min3;

			for (Int32 i = 0; i <= d.GetUpperBound(0); i += 1)
			{
				d[i, 0] = i;
			}

			for (Int32 i = 0; i <= d.GetUpperBound(1); i += 1)
			{
				d[0, i] = i;
			}

			for (Int32 i = 1; i <= d.GetUpperBound(0); i += 1)
			{
				for (Int32 j = 1; j <= d.GetUpperBound(1); j += 1)
				{
					cost = Convert.ToInt32(!(a[i - 1] == b[j - 1]));

					min1 = d[i - 1, j] + 1;
					min2 = d[i, j - 1] + 1;
					min3 = d[i - 1, j - 1] + cost;
					d[i, j] = Math.Min(Math.Min(min1, min2), min3);
				}
			}

			return d[d.GetUpperBound(0), d.GetUpperBound(1)];
		}

		public static byte[] ConvertStringToByteArray(string data, string fileName = null)
		{
			byte[] byteArray = null;
			if (!String.IsNullOrEmpty(data))
			{
				System.Text.UTF8Encoding encoding = new System.Text.UTF8Encoding();
				byteArray = encoding.GetBytes(data);

#if DEBUG
				if (!String.IsNullOrEmpty(fileName))
				{
					try
					{
						if (System.IO.File.Exists(fileName))
							System.IO.File.Delete(fileName);
						using (StreamWriter f = new StreamWriter(fileName))
						{
							f.Write(data);
						}
					}
					catch (Exception) { }
				}
#endif
			}
			return byteArray;
		}

		public static string ConvertByteArrayToString(byte[] data)
		{
			string text = UTF8Encoding.UTF8.GetString(data, 0, data.Length);
			return text;
			//System.Text.UTF8Encoding encoding = new System.Text.UTF8Encoding();
			//return encoding.GetString(data);
		}

#if! SILVERLIGHT
		public static string CalculateMD5Hash(string content)
		{
			byte[] byteContent = ConvertStringToByteArray(content);
			return CalculateMD5Hash(byteContent);
		}

		public static string CalculateMD5Hash(byte[] content)
		{
			// step 1, calculate MD5 hash from input
			System.Security.Cryptography.MD5 md5 = System.Security.Cryptography.MD5.Create();
			byte[] hash = md5.ComputeHash(content);

			//return ConvertByteArrayToString(hash);

			// step 2, convert byte array to hex string
			StringBuilder sb = new StringBuilder();
			for (int i = 0; i < hash.Length; i++)
				sb.Append(hash[i].ToString("X2"));
			return sb.ToString();
		}

		public static string CalculateSHA1(string content)
		{
			byte[] byteContent = ConvertStringToByteArray(content);
			return CalculateSHA1(byteContent);
		}

		public static string CalculateSHA1(byte[] content)
		{
			System.Security.Cryptography.SHA1 sha = new System.Security.Cryptography.SHA1CryptoServiceProvider();
			
			byte[] hash = sha.ComputeHash(content);
			
			//return ConvertByteArrayToString(hash);

			// step 2, convert byte array to hex string
			StringBuilder sb = new StringBuilder();
			for (int i = 0; i < hash.Length; i++)
				sb.Append(hash[i].ToString("X2"));
			return sb.ToString();
		}

		public static string ComputeSHA512(string s)
		{
			byte[] buffer = Encoding.UTF8.GetBytes(s);
			buffer = System.Security.Cryptography.SHA512Managed.Create().ComputeHash(buffer);
			return Convert.ToBase64String(buffer).Substring(0, 86); // strip padding
		}
#endif

		public static string TrimEnd(string input, string[] words, char[] charsToRemove, bool removeNumbers, bool removeSpaces = true, bool ignoreCase = true)
		{
			bool changed = true;

			string strChars = (charsToRemove != null ? charsToRemove.ToString() : "") + (removeNumbers ? "0123456789" : "") + (removeSpaces ? " " : "");
			char[] allCharsToRemove = strChars.ToCharArray();

			while (changed)
			{
				changed = false;
				// remove words
				foreach (string word in words)
				{
					if (input.EndsWith(word, ignoreCase ? StringComparison.InvariantCultureIgnoreCase : StringComparison.InvariantCulture))
					{
						input = input.Substring(0, input.Length - word.Length);
						changed = true;
					}
				}
				//remove any unwanted chars
				int oldLen = input.Length;
				input = input.TrimEnd(allCharsToRemove);
				if (input.Length < oldLen)
					changed = true;
			}
			return input;
		}

		public static string TrimStart(string input, string[] words, char[] charsToRemove, bool removeNumbers, bool removeSpaces = true, bool ignoreCase = true)
		{
			bool changed = true;

			string strChars = (charsToRemove != null ? charsToRemove.ToString() : "") + (removeNumbers ? "0123456789" : "") + (removeSpaces ? " " : "");
			char[] allCharsToRemove = strChars.ToCharArray();

			while (changed)
			{
				changed = false;
				// remove words
				foreach (string word in words)
				{
					if (input.StartsWith(word, ignoreCase ? StringComparison.InvariantCultureIgnoreCase : StringComparison.InvariantCulture))
					{
						input = input.Substring(word.Length);
						changed = true;
					}
				}
				//remove any unwanted chars
				int oldLen = input.Length;
				input = input.TrimStart(allCharsToRemove);
				if (input.Length < oldLen)
					changed = true;
			}
			return input;
		}

		public static MemoryStream GetStream(string data, System.Text.Encoding encoding = null)
		{
			if (encoding == null)
				encoding = new System.Text.UTF8Encoding();
			byte[] byteContent = encoding.GetBytes(data);
			return new MemoryStream(byteContent);
		}

		#region encryption 
		// Encrypt a byte array into a byte array using a key and an IV 
		public static byte[] Encrypt(byte[] clearData, byte[] Key, byte[] IV)
		{
			// Create a MemoryStream that is going to accept the encrypted bytes 
			MemoryStream ms = new MemoryStream();

			// Create a symmetric algorithm. 
			// We are going to use Rijndael because it is strong and available on all platforms. 
			// You can use other algorithms, to do so substitute the next line with something like 
			//                      TripleDES alg = TripleDES.Create(); 			
			Rijndael alg = Rijndael.Create();

			// Now set the key and the IV. 
			// We need the IV (Initialization Vector) because the algorithm is operating in its default 
			// mode called CBC (Cipher Block Chaining). The IV is XORed with the first block (8 byte) 
			// of the data before it is encrypted, and then each encrypted block is XORed with the 
			// following block of plaintext. This is done to make encryption more secure. 
			// There is also a mode called ECB which does not need an IV, but it is much less secure. 
			alg.Key = Key;
			alg.IV = IV;

			// Create a CryptoStream through which we are going to be pumping our data. 
			// CryptoStreamMode.Write means that we are going to be writing data to the stream 
			// and the output will be written in the MemoryStream we have provided. 
			CryptoStream cs = new CryptoStream(ms, alg.CreateEncryptor(), CryptoStreamMode.Write);

			// Write the data and make it do the encryption 
			cs.Write(clearData, 0, clearData.Length);

			// Close the crypto stream (or do FlushFinalBlock). 
			// This will tell it that we have done our encryption and there is no more data coming in, 
			// and it is now a good time to apply the padding and finalize the encryption process. 
			cs.Close();

			// Now get the encrypted data from the MemoryStream. 
			// Some people make a mistake of using GetBuffer() here, which is not the right way. 
			byte[] encryptedData = ms.ToArray();

			return encryptedData;
		}

		// Encrypt a string into a string using a password 
		//    Uses Encrypt(byte[], byte[], byte[]) 
		public static string Encrypt(string clearText, string Password)
		{
			// First we need to turn the input string into a byte array. 
			byte[] clearBytes = System.Text.Encoding.Unicode.GetBytes(clearText);

			// Then, we need to turn the password into Key and IV 
			// We are using salt to make it harder to guess our key using a dictionary attack - 
			// trying to guess a password by enumerating all possible words. 
			PasswordDeriveBytes pdb = new PasswordDeriveBytes(Password,
						new byte[] { 0x49, 0x76, 0x61, 0x6e, 0x20, 0x4d, 0x65, 0x64, 0x76, 0x65, 0x64, 0x65, 0x76 });

			// Now get the key/IV and do the encryption using the function that accepts byte arrays. 
			// Using PasswordDeriveBytes object we are first getting 32 bytes for the Key 
			// (the default Rijndael key length is 256bit = 32bytes) and then 16 bytes for the IV. 
			// IV should always be the block size, which is by default 16 bytes (128 bit) for Rijndael. 
			// If you are using DES/TripleDES/RC2 the block size is 8 bytes and so should be the IV size. 
			// You can also read KeySize/BlockSize properties off the algorithm to find out the sizes. 
			byte[] encryptedData = Encrypt(clearBytes, pdb.GetBytes(32), pdb.GetBytes(16));

			// Now we need to turn the resulting byte array into a string. 
			// A common mistake would be to use an Encoding class for that. It does not work 
			// because not all byte values can be represented by characters. 
			// We are going to be using Base64 encoding that is designed exactly for what we are 
			// trying to do. 
			return Convert.ToBase64String(encryptedData);
		}

		// Encrypt bytes into bytes using a password 
		//    Uses Encrypt(byte[], byte[], byte[]) 
		public static byte[] Encrypt(byte[] clearData, string Password)
		{
			// We need to turn the password into Key and IV. 
			// We are using salt to make it harder to guess our key using a dictionary attack - 
			// trying to guess a password by enumerating all possible words. 
			PasswordDeriveBytes pdb = new PasswordDeriveBytes(Password,
						new byte[] { 0x49, 0x76, 0x61, 0x6e, 0x20, 0x4d, 0x65, 0x64, 0x76, 0x65, 0x64, 0x65, 0x76 });

			// Now get the key/IV and do the encryption using the function that accepts byte arrays. 
			// Using PasswordDeriveBytes object we are first getting 32 bytes for the Key 
			// (the default Rijndael key length is 256bit = 32bytes) and then 16 bytes for the IV. 
			// IV should always be the block size, which is by default 16 bytes (128 bit) for Rijndael. 
			// If you are using DES/TripleDES/RC2 the block size is 8 bytes and so should be the IV size. 
			// You can also read KeySize/BlockSize properties off the algorithm to find out the sizes. 
			return Encrypt(clearData, pdb.GetBytes(32), pdb.GetBytes(16));

		}

		// Encrypt a file into another file using a password 
		public static void Encrypt(string fileIn, string fileOut, string Password)
		{
			// First we are going to open the file streams 
			FileStream fsIn = new FileStream(fileIn, FileMode.Open, FileAccess.Read);
			FileStream fsOut = new FileStream(fileOut, FileMode.OpenOrCreate, FileAccess.Write);

			// Then we are going to derive a Key and an IV from the Password and create an algorithm 
			PasswordDeriveBytes pdb = new PasswordDeriveBytes(Password,
						new byte[] { 0x49, 0x76, 0x61, 0x6e, 0x20, 0x4d, 0x65, 0x64, 0x76, 0x65, 0x64, 0x65, 0x76 });

			Rijndael alg = Rijndael.Create();

			alg.Key = pdb.GetBytes(32);
			alg.IV = pdb.GetBytes(16);

			// Now create a crypto stream through which we are going to be pumping data. 
			// Our fileOut is going to be receiving the encrypted bytes. 
			CryptoStream cs = new CryptoStream(fsOut, alg.CreateEncryptor(), CryptoStreamMode.Write);

			// Now will will initialize a buffer and will be processing the input file in chunks. 
			// This is done to avoid reading the whole file (which can be huge) into memory. 
			int bufferLen = 4096;
			byte[] buffer = new byte[bufferLen];
			int bytesRead;

			do
			{
				// read a chunk of data from the input file 
				bytesRead = fsIn.Read(buffer, 0, bufferLen);

				// encrypt it 
				cs.Write(buffer, 0, bytesRead);

			} while (bytesRead != 0);

			// close everything 
			cs.Close(); // this will also close the unrelying fsOut stream 
			fsIn.Close();
		}

		// Decrypt a byte array into a byte array using a key and an IV 
		public static byte[] Decrypt(byte[] cipherData, byte[] Key, byte[] IV)
		{
			// Create a MemoryStream that is going to accept the decrypted bytes 
			MemoryStream ms = new MemoryStream();

			// Create a symmetric algorithm. 
			// We are going to use Rijndael because it is strong and available on all platforms. 
			// You can use other algorithms, to do so substitute the next line with something like 
			//                      TripleDES alg = TripleDES.Create(); 
			Rijndael alg = Rijndael.Create();

			// Now set the key and the IV. 
			// We need the IV (Initialization Vector) because the algorithm is operating in its default 
			// mode called CBC (Cipher Block Chaining). The IV is XORed with the first block (8 byte) 
			// of the data after it is decrypted, and then each decrypted block is XORed with the previous 
			// cipher block. This is done to make encryption more secure. 
			// There is also a mode called ECB which does not need an IV, but it is much less secure. 
			alg.Key = Key;
			alg.IV = IV;

			// Create a CryptoStream through which we are going to be pumping our data. 
			// CryptoStreamMode.Write means that we are going to be writing data to the stream 
			// and the output will be written in the MemoryStream we have provided. 
			CryptoStream cs = new CryptoStream(ms, alg.CreateDecryptor(), CryptoStreamMode.Write);

			// Write the data and make it do the decryption 
			cs.Write(cipherData, 0, cipherData.Length);

			// Close the crypto stream (or do FlushFinalBlock). 
			// This will tell it that we have done our decryption and there is no more data coming in, 
			// and it is now a good time to remove the padding and finalize the decryption process. 
			cs.Close();

			// Now get the decrypted data from the MemoryStream. 
			// Some people make a mistake of using GetBuffer() here, which is not the right way. 
			byte[] decryptedData = ms.ToArray();

			return decryptedData;
		}

		// Decrypt a string into a string using a password 
		//    Uses Decrypt(byte[], byte[], byte[]) 
		public static string Decrypt(string cipherText, string Password)
		{
			// First we need to turn the input string into a byte array. 
			// We presume that Base64 encoding was used 
			byte[] cipherBytes = Convert.FromBase64String(cipherText);

			// Then, we need to turn the password into Key and IV 
			// We are using salt to make it harder to guess our key using a dictionary attack - 
			// trying to guess a password by enumerating all possible words. 
			PasswordDeriveBytes pdb = new PasswordDeriveBytes(Password,
						new byte[] { 0x49, 0x76, 0x61, 0x6e, 0x20, 0x4d, 0x65, 0x64, 0x76, 0x65, 0x64, 0x65, 0x76 });

			// Now get the key/IV and do the decryption using the function that accepts byte arrays. 
			// Using PasswordDeriveBytes object we are first getting 32 bytes for the Key 
			// (the default Rijndael key length is 256bit = 32bytes) and then 16 bytes for the IV. 
			// IV should always be the block size, which is by default 16 bytes (128 bit) for Rijndael. 
			// If you are using DES/TripleDES/RC2 the block size is 8 bytes and so should be the IV size. 
			// You can also read KeySize/BlockSize properties off the algorithm to find out the sizes. 
			byte[] decryptedData = Decrypt(cipherBytes, pdb.GetBytes(32), pdb.GetBytes(16));

			// Now we need to turn the resulting byte array into a string. 
			// A common mistake would be to use an Encoding class for that. It does not work 
			// because not all byte values can be represented by characters. 
			// We are going to be using Base64 encoding that is designed exactly for what we are 
			// trying to do. 
			return System.Text.Encoding.Unicode.GetString(decryptedData);

		}

		// Decrypt bytes into bytes using a password 
		//    Uses Decrypt(byte[], byte[], byte[]) 
		public static byte[] Decrypt(byte[] cipherData, string Password)
		{
			// We need to turn the password into Key and IV. 
			// We are using salt to make it harder to guess our key using a dictionary attack - 
			// trying to guess a password by enumerating all possible words. 
			PasswordDeriveBytes pdb = new PasswordDeriveBytes(Password,
						new byte[] { 0x49, 0x76, 0x61, 0x6e, 0x20, 0x4d, 0x65, 0x64, 0x76, 0x65, 0x64, 0x65, 0x76 });

			// Now get the key/IV and do the Decryption using the function that accepts byte arrays. 
			// Using PasswordDeriveBytes object we are first getting 32 bytes for the Key 
			// (the default Rijndael key length is 256bit = 32bytes) and then 16 bytes for the IV. 
			// IV should always be the block size, which is by default 16 bytes (128 bit) for Rijndael. 
			// If you are using DES/TripleDES/RC2 the block size is 8 bytes and so should be the IV size. 
			// You can also read KeySize/BlockSize properties off the algorithm to find out the sizes. 
			return Decrypt(cipherData, pdb.GetBytes(32), pdb.GetBytes(16));

		}

		// Decrypt a file into another file using a password 
		public static void Decrypt(string fileIn, string fileOut, string Password)
		{
			// First we are going to open the file streams 
			FileStream fsIn = new FileStream(fileIn, FileMode.Open, FileAccess.Read);
			FileStream fsOut = new FileStream(fileOut, FileMode.OpenOrCreate, FileAccess.Write);

			// Then we are going to derive a Key and an IV from the Password and create an algorithm 
			PasswordDeriveBytes pdb = new PasswordDeriveBytes(Password,
						new byte[] { 0x49, 0x76, 0x61, 0x6e, 0x20, 0x4d, 0x65, 0x64, 0x76, 0x65, 0x64, 0x65, 0x76 });

			Rijndael alg = Rijndael.Create();

			alg.Key = pdb.GetBytes(32);
			alg.IV = pdb.GetBytes(16);

			// Now create a crypto stream through which we are going to be pumping data. 
			// Our fileOut is going to be receiving the Decrypted bytes. 
			CryptoStream cs = new CryptoStream(fsOut, alg.CreateDecryptor(), CryptoStreamMode.Write);

			// Now will will initialize a buffer and will be processing the input file in chunks. 
			// This is done to avoid reading the whole file (which can be huge) into memory. 
			int bufferLen = 4096;
			byte[] buffer = new byte[bufferLen];
			int bytesRead;

			do
			{
				// read a chunk of data from the input file 
				bytesRead = fsIn.Read(buffer, 0, bufferLen);

				// Decrypt it 
				cs.Write(buffer, 0, bytesRead);

			} while (bytesRead != 0);

			// close everything 
			cs.Close(); // this will also close the unrelying fsOut stream 
			fsIn.Close();
		}
		#endregion
	}
	

}
