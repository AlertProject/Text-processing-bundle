/**************************************************************************\
    Copyright Microsoft Corporation. All Rights Reserved.
\**************************************************************************/

// This file contains general utilities to aid in development.
// Classes here generally shouldn't be exposed publicly since
// they're not particular to any library functionality.
// Because the classes here are internal, it's likely this file
// might be included in multiple assemblies.
namespace GenLib.General
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
    using System.Windows;
    using System.Windows.Media;
    using System.Windows.Media.Imaging;

    public enum SafeCopyFileOptions
    {
        PreserveOriginal,
        Overwrite,
        FindBetterName,
    }

    public static partial class Utility
    {
        public static string GetHashString(string value)
        {
            using (SHA1Managed sha = new SHA1Managed())
            {
				byte[] signatureHash = sha.ComputeHash(Encoding.UTF8.GetBytes(value));
                string signature = signatureHash.Aggregate(
                    new StringBuilder(),
                    (sb, b) => sb.Append(b.ToString("x2", CultureInfo.InvariantCulture))).ToString();
                return signature;
            }
        }
    }
}
