/**************************************************************************\
	Copyright Microsoft Corporation. All Rights Reserved.
\**************************************************************************/

// This file contains general utilities to aid in development.
// Classes here generally shouldn't be exposed publicly since
// they're not particular to any library functionality.
// Because the classes here are internal, it's likely this file
// might be included in multiple assemblies.
namespace GenLib
{
	using System;
	using System.Collections.Generic;
	using System.ComponentModel;
	using System.Diagnostics.CodeAnalysis;
	using System.Globalization;
	using System.IO;
	using System.Linq;
	using System.Reflection;
	using System.Runtime.InteropServices;
	using System.Security.Cryptography;
	using System.Text;
	//using System.Windows;
	//using System.Windows.Media;
	//using System.Windows.Media.Imaging;
	using GenLib.Misc;

	public static partial class Utility
	{
		public static readonly Version OsVersion = Environment.OSVersion.Version;
		public static readonly Random RandomNumberGenerator = new Random();

		[SuppressMessage("Microsoft.Performance", "CA1811:AvoidUncalledPrivateCode")]
		private static bool _MemCmp(IntPtr left, IntPtr right, long cb)
		{
			int offset = 0;

			for (; offset < (cb - sizeof(Int64)); offset += sizeof(Int64))
			{
				Int64 left64 = Marshal.ReadInt64(left, offset);
				Int64 right64 = Marshal.ReadInt64(right, offset);

				if (left64 != right64)
				{
					return false;
				}
			}

			for (; offset < cb; offset += sizeof(byte))
			{
				byte left8 = Marshal.ReadByte(left, offset);
				byte right8 = Marshal.ReadByte(right, offset);

				if (left8 != right8)
				{
					return false;
				}
			}

			return true;
		}

		[SuppressMessage("Microsoft.Performance", "CA1811:AvoidUncalledPrivateCode")]
		public static Exception FailableFunction<T>(Func<T> function, out T result)
		{
			return FailableFunction(5, function, out result);
		}

		[SuppressMessage("Microsoft.Performance", "CA1811:AvoidUncalledPrivateCode")]
		public static T FailableFunction<T>(Func<T> function)
		{
			T result;
			Exception e = FailableFunction(function, out result);
			if (e != null)
			{
				throw e;
			}
			return result;
		}

		[SuppressMessage("Microsoft.Performance", "CA1811:AvoidUncalledPrivateCode")]
		public static T FailableFunction<T>(int maxRetries, Func<T> function)
		{
			T result;
			Exception e = FailableFunction(maxRetries, function, out result);
			if (e != null)
			{
				throw e;
			}
			return result;
		}

		[SuppressMessage("Microsoft.Design", "CA1031:DoNotCatchGeneralExceptionTypes")]
		[SuppressMessage("Microsoft.Performance", "CA1811:AvoidUncalledPrivateCode")]
		public static Exception FailableFunction<T>(int maxRetries, Func<T> function, out T result)
		{
			//Assert.IsNotNull(function);
			//Assert.BoundedInteger(1, maxRetries, 100);
			int i = 0;
			while (true)
			{
				try
				{
					result = function();
					return null;
				}
				catch (Exception e)
				{
					if (i == maxRetries)
					{
						result = default(T);
						return e;
					}
				}
				++i;
			}
		}

		[SuppressMessage("Microsoft.Performance", "CA1811:AvoidUncalledPrivateCode")]
		public static int GET_X_LPARAM(IntPtr lParam)
		{
			return LOWORD(lParam.ToInt32());
		}

		[SuppressMessage("Microsoft.Performance", "CA1811:AvoidUncalledPrivateCode")]
		public static int GET_Y_LPARAM(IntPtr lParam)
		{
			return HIWORD(lParam.ToInt32());
		}

		[SuppressMessage("Microsoft.Performance", "CA1811:AvoidUncalledPrivateCode")]
		public static int HIWORD(int i)
		{
			return (short)(i >> 16);
		}

		[SuppressMessage("Microsoft.Performance", "CA1811:AvoidUncalledPrivateCode")]
		public static int LOWORD(int i)
		{
			return (short)(i & 0xFFFF);
		}

		[SuppressMessage("Microsoft.Performance", "CA1811:AvoidUncalledPrivateCode")]
		[SuppressMessage("Microsoft.Security", "CA2122:DoNotIndirectlyExposeMethodsWithLinkDemands")]
		public static bool AreStreamsEqual(Stream left, Stream right)
		{
			if (null == left)
			{
				return right == null;
			}
			if (null == right)
			{
				return false;
			}

			if (!left.CanRead || !right.CanRead)
			{
				throw new NotSupportedException("The streams can't be read for comparison");
			}

			if (left.Length != right.Length)
			{
				return false;
			}

			var length = (int)left.Length;

			// seek to beginning
			left.Position = 0;
			right.Position = 0;

			// total bytes read
			int totalReadLeft = 0;
			int totalReadRight = 0;

			// bytes read on this iteration
			int cbReadLeft = 0;
			int cbReadRight = 0;

			// where to store the read data
			var leftBuffer = new byte[512];
			var rightBuffer = new byte[512];

			// pin the left buffer
			GCHandle handleLeft = GCHandle.Alloc(leftBuffer, GCHandleType.Pinned);
			IntPtr ptrLeft = handleLeft.AddrOfPinnedObject();

			// pin the right buffer
			GCHandle handleRight = GCHandle.Alloc(rightBuffer, GCHandleType.Pinned);
			IntPtr ptrRight = handleRight.AddrOfPinnedObject();

			try
			{
				while (totalReadLeft < length)
				{
					//Assert.AreEqual(totalReadLeft, totalReadRight);

					cbReadLeft = left.Read(leftBuffer, 0, leftBuffer.Length);
					cbReadRight = right.Read(rightBuffer, 0, rightBuffer.Length);

					// verify the contents are an exact match
					if (cbReadLeft != cbReadRight)
					{
						return false;
					}

					if (!_MemCmp(ptrLeft, ptrRight, cbReadLeft))
					{
						return false;
					}

					totalReadLeft += cbReadLeft;
					totalReadRight += cbReadRight;
				}

				//Assert.AreEqual(cbReadLeft, cbReadRight);
				//Assert.AreEqual(totalReadLeft, totalReadRight);
				//Assert.AreEqual(length, totalReadLeft);

				return true;
			}
			finally
			{
				handleLeft.Free();
				handleRight.Free();
			}
		}

		[SuppressMessage("Microsoft.Performance", "CA1811:AvoidUncalledPrivateCode")]
		public static bool GuidTryParse(string guidString, out Guid guid)
		{
			Verify.IsNeitherNullNorEmpty(guidString, "guidString");

			try
			{
				guid = new Guid(guidString);
				return true;
			}
			catch (FormatException)
			{
			}
			catch (OverflowException)
			{
			}
			// Doesn't seem to be a valid guid.
			guid = default(Guid);
			return false;
		}

		[SuppressMessage("Microsoft.Performance", "CA1811:AvoidUncalledPrivateCode")]
		public static bool IsFlagSet(int value, int mask)
		{
			return 0 != (value & mask);
		}

		[SuppressMessage("Microsoft.Performance", "CA1811:AvoidUncalledPrivateCode")]
		public static bool IsFlagSet(uint value, uint mask)
		{
			return 0 != (value & mask);
		}

		[SuppressMessage("Microsoft.Performance", "CA1811:AvoidUncalledPrivateCode")]
		public static bool IsFlagSet(long value, long mask)
		{
			return 0 != (value & mask);
		}

		[SuppressMessage("Microsoft.Performance", "CA1811:AvoidUncalledPrivateCode")]
		public static bool IsFlagSet(ulong value, ulong mask)
		{
			return 0 != (value & mask);
		}

		[SuppressMessage("Microsoft.Performance", "CA1811:AvoidUncalledPrivateCode")]
		public static bool IsInterfaceImplemented(Type objectType, Type interfaceType)
		{
			Assert.IsNotNull(objectType);
			Assert.IsNotNull(interfaceType);
			Assert.IsTrue(interfaceType.IsInterface);

			return objectType.GetInterfaces().Any(type => type == interfaceType);
		}

		[SuppressMessage("Microsoft.Performance", "CA1811:AvoidUncalledPrivateCode")]
		public static bool IsOSVistaOrNewer
		{
			get { return OsVersion >= new Version(6, 0); }
		}

		[SuppressMessage("Microsoft.Performance", "CA1811:AvoidUncalledPrivateCode")]
		public static bool IsOSWindows7OrNewer
		{
			get { return OsVersion >= new Version(6, 1); }
		}


		[SuppressMessage("Microsoft.Performance", "CA1811:AvoidUncalledPrivateCode")]
		public static void SafeDestroyIcon(ref IntPtr hicon)
		{
			IntPtr p = hicon;
			hicon = IntPtr.Zero;
			if (IntPtr.Zero != p)
			{
				NativeMethods.DestroyIcon(p);
			}
		}

		/// <summary>GDI's DeleteObject</summary>
		[SuppressMessage("Microsoft.Performance", "CA1811:AvoidUncalledPrivateCode")]
		public static void SafeDeleteObject(ref IntPtr gdiObject)
		{
			IntPtr p = gdiObject;
			gdiObject = IntPtr.Zero;
			if (IntPtr.Zero != p)
			{
				NativeMethods.DeleteObject(p);
			}
		}

		[SuppressMessage("Microsoft.Performance", "CA1811:AvoidUncalledPrivateCode")]
		public static void SafeDestroyWindow(ref IntPtr hwnd)
		{
			IntPtr p = hwnd;
			hwnd = IntPtr.Zero;
			if (NativeMethods.IsWindow(p))
			{
				NativeMethods.DestroyWindow(p);
			}
		}


		[SuppressMessage("Microsoft.Performance", "CA1811:AvoidUncalledPrivateCode")]
		public static void SafeDispose<T>(ref T disposable) where T : IDisposable
		{
			// Dispose can safely be called on an object multiple times.
			IDisposable t = disposable;
			disposable = default(T);
			if (null != t)
			{
				t.Dispose();
			}
		}

		/// <summary>GDI+'s DisposeImage</summary>
		/// <param name="gdipImage"></param>
		[SuppressMessage("Microsoft.Performance", "CA1811:AvoidUncalledPrivateCode")]
		public static void SafeDisposeImage(ref IntPtr gdipImage)
		{
			IntPtr p = gdipImage;
			gdipImage = IntPtr.Zero;
			if (IntPtr.Zero != p)
			{
				NativeMethods.GdipDisposeImage(p);
			}
		}

		[SuppressMessage("Microsoft.Performance", "CA1811:AvoidUncalledPrivateCode")]
		[SuppressMessage("Microsoft.Security", "CA2122:DoNotIndirectlyExposeMethodsWithLinkDemands")]
		public static void SafeCoTaskMemFree(ref IntPtr ptr)
		{
			IntPtr p = ptr;
			ptr = IntPtr.Zero;
			if (IntPtr.Zero != p)
			{
				Marshal.FreeCoTaskMem(p);
			}
		}

		[SuppressMessage("Microsoft.Performance", "CA1811:AvoidUncalledPrivateCode")]
		[SuppressMessage("Microsoft.Security", "CA2122:DoNotIndirectlyExposeMethodsWithLinkDemands")]
		public static void SafeFreeHGlobal(ref IntPtr hglobal)
		{
			IntPtr p = hglobal;
			hglobal = IntPtr.Zero;
			if (IntPtr.Zero != p)
			{
				Marshal.FreeHGlobal(p);
			}
		}

		[SuppressMessage("Microsoft.Performance", "CA1811:AvoidUncalledPrivateCode")]
		[SuppressMessage("Microsoft.Security", "CA2122:DoNotIndirectlyExposeMethodsWithLinkDemands")]
		public static void SafeRelease<T>(ref T comObject) where T : class
		{
			T t = comObject;
			comObject = default(T);
			if (null != t)
			{
				Assert.IsTrue(Marshal.IsComObject(t));
				Marshal.ReleaseComObject(t);
			}
		}

		/// <summary>
		/// Utility to help classes catenate their properties for implementing ToString().
		/// </summary>
		/// <param name="source">The StringBuilder to catenate the results into.</param>
		/// <param name="propertyName">The name of the property to be catenated.</param>
		/// <param name="value">The value of the property to be catenated.</param>
		[SuppressMessage("Microsoft.Performance", "CA1811:AvoidUncalledPrivateCode")]
		public static void GeneratePropertyString(StringBuilder source, string propertyName, string value)
		{
			Assert.IsNotNull(source);
			Assert.IsFalse(string.IsNullOrEmpty(propertyName));

			if (0 != source.Length)
			{
				source.Append(' ');
			}

			source.Append(propertyName);
			source.Append(": ");
			if (string.IsNullOrEmpty(value))
			{
				source.Append("<null>");
			}
			else
			{
				source.Append('\"');
				source.Append(value);
				source.Append('\"');
			}
		}

		/// <summary>
		/// Generates ToString functionality for a struct.  This is an expensive way to do it,
		/// it exists for the sake of debugging while classes are in flux.
		/// Eventually this should just be removed and the classes should
		/// do this without reflection.
		/// </summary>
		/// <typeparam name="T"></typeparam>
		/// <param name="object"></param>
		/// <returns></returns>
		[SuppressMessage("Microsoft.Performance", "CA1811:AvoidUncalledPrivateCode")]
		[Obsolete]
		public static string GenerateToString<T>(T @object) where T : struct
		{
			var sbRet = new StringBuilder();
			foreach (PropertyInfo property in typeof(T).GetProperties(BindingFlags.Public | BindingFlags.Instance))
			{
				if (0 != sbRet.Length)
				{
					sbRet.Append(", ");
				}
				Assert.AreEqual(0, property.GetIndexParameters().Length);
				object value = property.GetValue(@object, null);
				string format = null == value ? "{0}: <null>" : "{0}: \"{1}\"";
				sbRet.AppendFormat(format, property.Name, value);
			}
			return sbRet.ToString();
		}

		[SuppressMessage("Microsoft.Performance", "CA1811:AvoidUncalledPrivateCode")]
		public static void CopyStream(Stream destination, Stream source)
		{
			Assert.IsNotNull(source);
			Assert.IsNotNull(destination);

			destination.Position = 0;

			// If we're copying from, say, a web stream, don't fail because of this.
			if (source.CanSeek)
			{
				source.Position = 0;

				// Consider that this could throw because 
				// the source stream doesn't know it's size...
				destination.SetLength(source.Length);
			}

			var buffer = new byte[4096];
			int cbRead;

			do
			{
				cbRead = source.Read(buffer, 0, buffer.Length);
				if (0 != cbRead)
				{
					destination.Write(buffer, 0, cbRead);
				}
			}
			while (buffer.Length == cbRead);

			// Reset the Seek pointer before returning.
			destination.Position = 0;
		}

		[SuppressMessage("Microsoft.Performance", "CA1811:AvoidUncalledPrivateCode")]
		public static string HashStreamMD5(Stream stm)
		{
			stm.Position = 0;
			var hashBuilder = new StringBuilder();
			using (MD5 md5 = MD5.Create())
			{
				foreach (byte b in md5.ComputeHash(stm))
				{
					hashBuilder.Append(b.ToString("x2", CultureInfo.InvariantCulture));
				}
			}

			return hashBuilder.ToString();
		}


		[SuppressMessage("Microsoft.Performance", "CA1811:AvoidUncalledPrivateCode")]
		public static bool MemCmp(byte[] left, byte[] right, int cb)
		{
			Assert.IsNotNull(left);
			Assert.IsNotNull(right);

			Assert.IsTrue(cb <= Math.Min(left.Length, right.Length));

			// pin this buffer
			GCHandle handleLeft = GCHandle.Alloc(left, GCHandleType.Pinned);
			IntPtr ptrLeft = handleLeft.AddrOfPinnedObject();

			// pin the other buffer
			GCHandle handleRight = GCHandle.Alloc(right, GCHandleType.Pinned);
			IntPtr ptrRight = handleRight.AddrOfPinnedObject();

			bool fRet = _MemCmp(ptrLeft, ptrRight, cb);

			handleLeft.Free();
			handleRight.Free();

			return fRet;
		}
		
		public static bool TryFileMove(string sourceFileName, string destFileName)
		{
			if (!File.Exists(destFileName))
			{
				try
				{
					File.Move(sourceFileName, destFileName);
				}
				catch (IOException)
				{
					return false;
				}
				return true;
			}
			return false;
		}


	}
}
