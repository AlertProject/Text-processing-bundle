using System;
using System.Collections.Generic;
using System.Text;
using System.Runtime.InteropServices;
using System.Collections;


namespace ItemMinerControl
{
	public class ItemMiner
	{
		private static System.Text.UTF8Encoding _utf8Enc = new UTF8Encoding();
		private static UTF8Marshaler _utf8Marsh = new UTF8Marshaler();

		private class HMailMiner
		{
			[DllImport("ItemMinerLib")]
			public static extern bool DllValid();

			[DllImport("ItemMinerLib")]
			public static extern void DelCStr(IntPtr CStr);

			[DllImport("ItemMinerLib")]
			public static extern void SetVerbosity(int VerbosityLev);

			[DllImport("ItemMinerLib")]
			public static extern IntPtr GetLastInformation(int ProfileHnd);

			[DllImport("ItemMinerLib")]
			public static extern IntPtr GetStatus(int ProfileHnd);

			[DllImport("ItemMinerLib")]
			public static extern int GetVerbosity();

			[DllImport("ItemMinerLib")]
			public static extern int ProfileNew(String ProfilePath, String UnicodeDefFile, int MxNGramLen, int MxCachedNGrams, int IndexCacheSizeMB, int ItemCacheSizeMB);

			[DllImport("ItemMinerLib")]
			public static extern int ProfileLoad(String ProfilePath, String UnicodeDefFile, int FAccess, int IndexCacheSizeMB, int ItemCacheSizeMB);

			[DllImport("ItemMinerLib")]
			public static extern void ProfileClose(int ProfileHnd);

			[DllImport("ItemMinerLib")]
			public static extern void ClearResults(int ProfileHnd);
			
			[DllImport("ItemMinerLib")]
			public static extern IntPtr Query(int ProfileHnd, byte[] QueryInfo);

			[DllImport("ItemMinerLib")]
			public static extern void RemoveItem(int ProfileHnd, int ItemId);

			[DllImport("ItemMinerLib")]
			public static extern void RemoveItems(int ProfileHnd, byte[] RemoveContent);

			[DllImport("ItemMinerLib")]
			public static extern bool SetTagData(int ProfileHnd, byte[] TagData);

			[DllImport("ItemMinerLib")]
			public static extern IntPtr GetTagData(int ProfileHnd, byte[] TagData);

			[DllImport("ItemMinerLib")]
			public static extern IntPtr AddItem(int ProfileHnd, byte[] ItemInfo, byte[] ItemContent);

			[DllImport("ItemMinerLib")]
			public static extern IntPtr UpdateItem(int ProfileHnd, byte[] ItemInfo, byte[] ItemContent);

			[DllImport("ItemMinerLib")]
			public static extern void SetTag(int ProfileHnd, int ItemId, string TagId);

			[DllImport("ItemMinerLib")]
			public static extern void RemoveTag(int ProfileHnd, int ItemId, string TagId);

			[DllImport("ItemMinerLib")]
			public static extern IntPtr GetItemContent(int ProfileHnd, byte[] QueryInfo);

			[DllImport("ItemMinerLib")]
			public static extern IntPtr GetItem(int ProfileHnd, byte[] QueryInfo);

			[DllImport("ItemMinerLib")]
			public static extern IntPtr GetTopWords(int ProfileHnd, int keywordCount, bool groupByThreads, int maxNGramLen, int minNGramFq);

			[DllImport("ItemMinerLib")]
			public static extern IntPtr ExecuteCommand(int ProfileHnd, byte[] Command);

			[DllImport("ItemMinerLib")]
			public static extern UInt64 GetTmFromStr(int Year, int Month, int Day, int Hour, int Minute, int Seconds);

			[DllImport("ItemMinerLib")]
			public static extern IntPtr TokenizeText(int ProfileHnd, byte[] Text);

			[DllImport("ItemMinerLib")]
			public static extern void UpdateSettings(int ProfileHnd, byte[] Text);
		}

		public static bool DllValid()
		{
			return HMailMiner.DllValid();
		}

		public static int ProfileNew(String ProfilePath, String UnicodeDefFile, int MxNGramLen, int MxCachedNGrams, int IndexCacheSizeMB, int ItemCacheSizeMB)
		{
			return HMailMiner.ProfileNew(ProfilePath, UnicodeDefFile, MxNGramLen, MxCachedNGrams, IndexCacheSizeMB, ItemCacheSizeMB);
		}

		public static int ProfileLoad(String ProfilePath, String UnicodeDefFile, int FAccess, int IndexCacheSizeMB, int ItemCacheSizeMB)
		{
			return HMailMiner.ProfileLoad(ProfilePath, UnicodeDefFile, FAccess, IndexCacheSizeMB, ItemCacheSizeMB);
		}

		public static void ProfileClose(int ProfileHnd)
		{
			HMailMiner.ProfileClose(ProfileHnd);
		}

		public static void ClearResults(int ProfileHnd)
		{
			HMailMiner.ClearResults(ProfileHnd);
		}

		public static String GetLastInformation(int ProfileHnd)
		{
			IntPtr i = HMailMiner.GetLastInformation(ProfileHnd);
			string s = (string)_utf8Marsh.MarshalNativeToManaged(i);
			HMailMiner.DelCStr(i);
			return s;
		}

		public static String GetStatus(int ProfileHnd)
		{
			IntPtr i = HMailMiner.GetStatus(ProfileHnd);
			string s = (string)_utf8Marsh.MarshalNativeToManaged(i);
			HMailMiner.DelCStr(i);
			return s;
		}
		

		public static String Query(int ProfileHnd, String QueryInfo)
		{
			byte[] queryInfo = _utf8Enc.GetBytes(QueryInfo);
			IntPtr i = HMailMiner.Query(ProfileHnd, queryInfo);
			string s = (string)_utf8Marsh.MarshalNativeToManaged(i);
			HMailMiner.DelCStr(i);
			return s;
		}

		public static void RemoveItem(int ProfileHnd, int ItemId)
		{
			HMailMiner.RemoveItem(ProfileHnd, ItemId);
		}

		public static void RemoveItems(int ProfileHnd, string RemoveContent)
		{
			byte[] removeContent = _utf8Enc.GetBytes(RemoveContent);
			HMailMiner.RemoveItems(ProfileHnd, removeContent);
		}

		public static bool SetTagData(int ProfileHnd, string TagData)
		{
			byte[] tagData = _utf8Enc.GetBytes(TagData);
			return HMailMiner.SetTagData(ProfileHnd, tagData);
		}

		public static String GetTagData(int ProfileHnd, string TagData)
		{
			byte[] tagData = _utf8Enc.GetBytes(TagData);
			IntPtr i = HMailMiner.GetTagData(ProfileHnd, tagData);
			string s = (string)_utf8Marsh.MarshalNativeToManaged(i);
			HMailMiner.DelCStr(i);
			return s;
		}

		public static String AddItem(int ProfileHnd, String ItemInfo, String ItemContent)
		{
			// convert string to utf-8 array
			byte[] itemInfo = _utf8Enc.GetBytes(ItemInfo);
			byte[] itemContent = _utf8Enc.GetBytes(ItemContent);
			IntPtr i = HMailMiner.AddItem(ProfileHnd, itemInfo, itemContent);
			string s = (string)_utf8Marsh.MarshalNativeToManaged(i);
			HMailMiner.DelCStr(i);
			return s;
		}

		public static String UpdateItem(int ProfileHnd, String ItemInfo, String ItemContent)
		{
			// convert string to utf-8 array
			byte[] itemInfo = _utf8Enc.GetBytes(ItemInfo);
			byte[] itemContent = _utf8Enc.GetBytes(ItemContent);
			IntPtr i = HMailMiner.UpdateItem(ProfileHnd, itemInfo, itemContent);
			string s = (string)_utf8Marsh.MarshalNativeToManaged(i);
			HMailMiner.DelCStr(i);
			return s;
		}

		public static void SetTag(int ProfileHnd, int ItemId, string TagId)
		{
			HMailMiner.SetTag(ProfileHnd, ItemId, TagId);
		}
		
		public static void RemoveTag(int ProfileHnd, int ItemId, string TagId)
		{
			HMailMiner.RemoveTag(ProfileHnd, ItemId, TagId);
		}

		public static String GetItemContent(int ProfileHnd, string QueryInfo)
		{
			byte[] queryInfo = _utf8Enc.GetBytes(QueryInfo);
			IntPtr i = HMailMiner.GetItemContent(ProfileHnd, queryInfo);
			string s = (string)_utf8Marsh.MarshalNativeToManaged(i);
			HMailMiner.DelCStr(i);
			return s;
		}

		public static String GetItem(int ProfileHnd, String QueryInfo)
		{
			byte[] queryInfo = _utf8Enc.GetBytes(QueryInfo);
			IntPtr i = HMailMiner.GetItem(ProfileHnd, queryInfo);
			string s = (string)_utf8Marsh.MarshalNativeToManaged(i);
			HMailMiner.DelCStr(i);
			return s;
		}

		public static String GetTopWords(int ProfileHnd, int keywordCount, bool groupByThreads, int maxNGramLen, int minNGramFq)
		{
			IntPtr i = HMailMiner.GetTopWords(ProfileHnd, keywordCount, groupByThreads, maxNGramLen, minNGramFq);
			string s = (string)_utf8Marsh.MarshalNativeToManaged(i);
			HMailMiner.DelCStr(i);
			return s;
		}

		public static String ExecuteCommand(int ProfileHnd, string Command)
		{
			byte[] command = _utf8Enc.GetBytes(Command);
			IntPtr i = HMailMiner.ExecuteCommand(ProfileHnd, command);
			string s = (string)_utf8Marsh.MarshalNativeToManaged(i);
			HMailMiner.DelCStr(i);
			return s;
		}

		public static String TokenizeText(int ProfileHnd, string Text)
		{
			byte[] text = _utf8Enc.GetBytes(Text);
			IntPtr i = HMailMiner.TokenizeText(ProfileHnd, text);
			string s = (string)_utf8Marsh.MarshalNativeToManaged(i);
			HMailMiner.DelCStr(i);
			return s;
		}

		public static void UpdateSettings(int ProfileHnd, string Settings)
		{
			byte[] settings = _utf8Enc.GetBytes(Settings);
			HMailMiner.UpdateSettings(ProfileHnd, settings);
		}

		public static UInt64 GetTmFromStr(int Year, int Month, int Day, int Hour, int Minute, int Seconds)
		{
			return HMailMiner.GetTmFromStr(Year, Month, Day, Hour, Minute, Seconds);
		}
	}

	public class UTF8Marshaler : ICustomMarshaler
	{
		static UTF8Marshaler marshaler = new UTF8Marshaler();

		private Hashtable allocated = new Hashtable();

		public static ICustomMarshaler GetInstance(string cookie)
		{
			return marshaler;
		}

		public void CleanUpManagedData(object ManagedObj)
		{
		}

		public void CleanUpNativeData(IntPtr pNativeData)
		{
			/* This is a hack not to crash on mono!!! */
			if (allocated.Contains(pNativeData))
			{
				Marshal.FreeHGlobal(pNativeData);
				allocated.Remove(pNativeData);
			}
			else
			{
				Console.WriteLine("WARNING: Trying to free an unallocated pointer!");
				Console.WriteLine("         This is most likely a bug in mono");
			}
		}

		public int GetNativeDataSize()
		{
			return -1;
		}

		public IntPtr MarshalManagedToNative(object ManagedObj)
		{
			if (ManagedObj == null)
				return IntPtr.Zero;
			if (ManagedObj.GetType() != typeof(string))
				throw new ArgumentException("ManagedObj", "Can only marshal type of System.string");

			byte[] array = Encoding.UTF8.GetBytes((string)ManagedObj);
			int size = Marshal.SizeOf(typeof(byte)) * (array.Length + 1);

			IntPtr ptr = Marshal.AllocHGlobal(size);

			/* This is a hack not to crash on mono!!! */
			allocated.Add(ptr, null);

			Marshal.Copy(array, 0, ptr, array.Length);
			Marshal.WriteByte(ptr, array.Length, 0);

			return ptr;
		}

		public object MarshalNativeToManaged(IntPtr pNativeData)
		{
			if (pNativeData == IntPtr.Zero)
				return null;

			int size = 0;
			while (Marshal.ReadByte(pNativeData, size) > 0)
				size++;

			byte[] array = new byte[size];
			Marshal.Copy(pNativeData, array, 0, size);

			return Encoding.UTF8.GetString(array);
		}
	}
}
