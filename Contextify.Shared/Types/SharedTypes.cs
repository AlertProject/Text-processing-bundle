using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;

namespace Contextify.Shared.Types
{
	public static class Consts
	{
		public const string AnyRole = "_ANY_";
	}

	public enum ESQLFolderState
	{
		NotIndexed = 0,
		Indexed,
		ToBeAdded,
		ToBeRemoved
	}

	public enum ESQLItemState
	{
		NotIndexed = 0,
		Indexed,
		ToBeAdded
	}

	public enum ESQLItemType
	{
		Email = 0,
		Attachment,
		Document,
		FBWallPost,
		FBMessage,
		Tweet
	}

	public enum EAccountType { Invalid = 0, Email, Facebook, LinkedIn, Twitter, Outlook, Custom };

	public enum EPersonNameTrust : short
	{
		Invalid = 0,
		Low = 1,		// if we get this name from from an email where the person was a recipient
		Med,			// if we get this name from from an email where the person was the sender
		High			// if we manually set this name or we got it from outlook contacts
	}
}
