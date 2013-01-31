using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Windows;
using System.Windows.Media;
using System.IO;
using System.Diagnostics;
using System.Management; //Be sure to add this as a Reference, also.

namespace GenLib.Sys
{
	/// <summary>
	/// Object returned by the call to CUserDomainSid.GetUDS()
	/// </summary>
	/// 
	public class UdsObj
	{
		/// <summary>
		/// Domain prefix for the user
		/// </summary>
		public string Domain {  get; set; }

		/// <summary>
		/// User ID
		/// </summary>
		public string User { get; set; }

		/// <summary>
		/// SID for the user
		/// </summary>
		public string SID { get; set; }

		/// <summary>
		/// Constructor
		/// </summary>
		public UdsObj()
		{
			Domain = "";
			User = "";
			SID = "";
		}
	}
   
	/// <summary>
	/// Extracts the User, Domain and SID of the current user.
	/// </summary>
	public class UserDomainSid
	{
		/// <summary>
		/// The root scope of the query.
		/// </summary>
		private static string strScope = @"\\.\root\cimv2";
		/// <summary>
		/// Use the System.Management namespace to get the current User, Domain and SID.
		/// This is useful for querying and setting certain values in the system registry.
		/// </summary>
		/// <returns>CUserDomainSid</returns>
		/// Problems: The code does not work if you run the application through Remote desktop - you don't get an empty username
		public static UdsObj GetUDS()
		{
			UdsObj udsRetVal = new UdsObj();
			//
			// UserName will contain the domain and the user
			string strQuery = "Select UserName FROM Win32_ComputerSystem";

			///////////////////////////////////////////////////////
			// Query the ManagementObject to find the current user
			using (ManagementObjectSearcher mosItems = new ManagementObjectSearcher(strScope, strQuery))
			{
				foreach (ManagementBaseObject mbo in mosItems.Get())
				{  //ManagementObjectCollection
					foreach (PropertyData property in mbo.Properties)
					{
						if (property.Name != "UserName") continue;
						if (property.Value == null) continue;
						string[] arr_strUserDom = property.Value.ToString().Split('\\');
						udsRetVal.Domain = arr_strUserDom[0];
						udsRetVal.User = arr_strUserDom[1];
					}
				}
			}

			udsRetVal.SID =
				GetSid(udsRetVal.Domain, udsRetVal.User);
         
			return udsRetVal;
		}

		/// <summary>
		/// GetSid uses the System.Management namespace to obtain the SID 
		/// for a given domain and user name
		/// </summary>
		/// <param name="strDomain">the user's domain</param>
		/// <param name="strUser">the user's login ID</param>
		/// <returns>string (SID)</returns>
		public static string GetSid(string strDomain, string strUser)
		{
			string strRetValSid = "";
			///////////////////////////////////////////////////////////////
			// Create a new query into the Win32_UserAccount to get the SID
			
			// Go directly to the SID bypassing all others
			string strQuery = string.Format("Select SID FROM Win32_UserAccount WHERE Domain='{0:G}' AND Name='{1:G}'", strDomain, strUser);
			
			/////////////////////////////////////////////////////////////
			// Run the second query once we have the user name and Domain
			using (ManagementObjectSearcher mosItems = new ManagementObjectSearcher(strScope, strQuery))
			{  
				//ManagementObjectCollection
				foreach (ManagementBaseObject mbo in mosItems.Get())
				{  // Go directly to the SID bypassing all others
					strRetValSid = mbo["SID"].ToString();
				}
			}

			return strRetValSid;
		}
	}

	public static class General
	{
		public static string GetCurrentUser()
		{
			try 
			{
				return System.Environment.UserName;
			}
			catch
			{
				return null;
			}
		}
	}
}
