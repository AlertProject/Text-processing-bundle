using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Diagnostics.CodeAnalysis;
using System.IO;
using GenLib.Misc;

namespace GenLib
{
	public class IO
	{
		public enum SafeCopyFileOptions
		{
			PreserveOriginal,
			Overwrite,
			FindBetterName,
		}

		[SuppressMessage("Microsoft.Performance", "CA1811:AvoidUncalledPrivateCode")]
		public static void EnsureDirectory(string path)
		{
			if (!path.EndsWith(@"\"))
				path += @"\";

			path = Path.GetDirectoryName(path);

			if (!Directory.Exists(path))
				Directory.CreateDirectory(path);
		}

		public static bool WriteFile(string filename, Object content)
		{
			try
			{
				using (StreamWriter outfile = new StreamWriter(filename))
				{
					outfile.Write(content.ToString());
				}
				return true;
			}
			catch (Exception ex)
			{
				System.Diagnostics.Trace.WriteLine(String.Format("Unable to write file {0} because of error: {1}", filename, ex.Message));
				return false;
			}
		}

		/// <summary>
		/// Simple guard against the exceptions that File.Delete throws on null and empty strings.
		/// </summary>
		/// <param name="path">The path to delete.  Unlike File.Delete, this can be null or empty.</param>
		/// <remarks>
		/// Note that File.Delete, and by extension SafeDeleteFile, does not throw an exception
		/// if the file does not exist.
		/// </remarks>
		[SuppressMessage("Microsoft.Performance", "CA1811:AvoidUncalledPrivateCode")]
		public static bool SafeDeleteFile(string path)
		{
			if (System.IO.File.Exists(path))
			{
				try
				{
					System.IO.File.Delete(path);
					return true;
				}
				catch { return false; }
			}
			else
				return false;
		}


		/// <summary>
		/// Wrapper around File.Copy to provide feedback as to whether the file wasn't copied because it didn't exist.
		/// </summary>
		/// <param name="cachePath"></param>
		/// <param name="suggestedPath"></param>
		/// <param name="overwrite"></param>
		/// <returns></returns>
		public static string SafeCopyFile(string sourceFileName, string destFileName, SafeCopyFileOptions options)
		{
			switch (options)
			{
				case SafeCopyFileOptions.PreserveOriginal:
					if (!File.Exists(destFileName))
					{
						File.Copy(sourceFileName, destFileName);
						return destFileName;
					}
					return null;
				case SafeCopyFileOptions.Overwrite:
					File.Copy(sourceFileName, destFileName, true);
					return destFileName;
				case SafeCopyFileOptions.FindBetterName:
					string directoryPart = Path.GetDirectoryName(destFileName);
					string fileNamePart = Path.GetFileNameWithoutExtension(destFileName);
					string extensionPart = Path.GetExtension(destFileName);
					foreach (string path in GenerateFileNames(directoryPart, fileNamePart, extensionPart))
					{
						if (!File.Exists(path))
						{
							File.Copy(sourceFileName, path);
							return path;
						}
					}
					return null;
			}
			throw new ArgumentException("Invalid enumeration value", "options");
		}

		public static IEnumerable<string> GenerateFileNames(string directory, string primaryFileName, string extension)
		{
			Verify.IsNeitherNullNorEmpty(directory, "directory");
			Verify.IsNeitherNullNorEmpty(primaryFileName, "primaryFileName");

			primaryFileName = MakeValidFileName(primaryFileName);

			for (int i = 0; i <= 50; ++i)
			{
				if (0 == i)
				{
					yield return Path.Combine(directory, primaryFileName) + extension;
				}
				else if (40 >= i)
				{
					yield return Path.Combine(directory, primaryFileName) + " (" + i.ToString((IFormatProvider)null) + ")" + extension;
				}
				else
				{
					// At this point we're hitting pathological cases.  This should stir things up enough that it works.
					// If this fails because of naming conflicts after an extra 10 tries, then I don't care.
					yield return Path.Combine(directory, primaryFileName) + " (" + GenLib.Utility.RandomNumberGenerator.Next(41, 9999) + ")" + extension;
				}
			}
		}

		public static string MakeValidFileName(string invalidPath)
		{
			return invalidPath
				.Replace('\\', '_')
				.Replace('/', '_')
				.Replace(':', '_')
				.Replace('*', '_')
				.Replace('?', '_')
				.Replace('\"', '_')
				.Replace('<', '_')
				.Replace('>', '_')
				.Replace('|', '_');
		}

	}
}
