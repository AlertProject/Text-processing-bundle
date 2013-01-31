
namespace GenLib.Web
{
    using System;
    using System.Diagnostics;
    using System.Text;

    [DebuggerDisplay("SmallUri: { GetUri() }")]
    public struct SmallUri : IEquatable<SmallUri>
    {
        private static readonly UTF8Encoding s_Encoder = new UTF8Encoding(false /* do not emit BOM */, true /* throw on error */);
        private readonly byte[] _utf8String;
        private readonly bool _isHttp;

        public SmallUri(Uri value)
        {
            _isHttp = false;
            _utf8String = null;

            if (value == null)
            {
                return;
            }

            if (!value.IsAbsoluteUri)
            {
                throw new ArgumentException("The parameter is not a valid absolute uri", "value");
            }

            string strValue = value.OriginalString;
            if (strValue.StartsWith("http://", StringComparison.OrdinalIgnoreCase))
            {
                _isHttp = true;
                strValue = strValue.Substring(7);
            }

            _utf8String = s_Encoder.GetBytes(strValue);
        }

        public SmallUri(string value)
        {
            _isHttp = false;
            _utf8String = null;

            if (string.IsNullOrEmpty(value))
            {
                return;
            }

            if (!Uri.IsWellFormedUriString(value, UriKind.Absolute))
            {
                throw new ArgumentException("The parameter is not a valid uri", "value");
            }
            
            if (value.StartsWith("http://", StringComparison.OrdinalIgnoreCase))
            {
                _isHttp = true;
                value = value.Substring(7);
            }
            
            _utf8String = s_Encoder.GetBytes(value);
        }

        #region Object Overrides

        public override string ToString()
        {
			return GetString();
        }

        public override int GetHashCode()
        {
            // Intentionally hashes similarly to the expanded strings.
            return GetString().GetHashCode();
        }

        public override bool Equals(object obj)
        {
            try
            {
                return Equals((SmallUri)obj);
            }
            catch (InvalidCastException)
            {
                return false;
            }
        }

        #endregion

        #region IEquatable<SmallUri> Members


        public bool Equals(SmallUri other)
        {
            if (_utf8String == null)
            {
                return other._utf8String == null;
            }

            if (other._utf8String == null)
            {
                return false;
            }

            if (_isHttp != other._isHttp)
            {
                return false;
            }

            if (_utf8String.Length != other._utf8String.Length)
            {
                return false;
            }
#if SILVERLIGHT
			return GetString() == other.GetString();
#else
			return GenLib.Utility.MemCmp(_utf8String, other._utf8String, _utf8String.Length);
#endif
		}

        #endregion

        public string GetString()
        {
            if (_utf8String == null)
            {
                return "";
            }
            return GetUri().ToString();
        }

        public Uri GetUri()
        {
            if (_utf8String == null)
            {
                return null;
            }
#if SILVERLIGHT
			return new Uri((_isHttp ? "http://" : "") + UTF8Encoding.UTF8.GetString(_utf8String, 0, _utf8String.Length), UriKind.Absolute);
#else
			return new Uri((_isHttp ? "http://" : "") + s_Encoder.GetString(_utf8String), UriKind.Absolute);
#endif
		}

        public static bool operator ==(SmallUri left, SmallUri right)
        {
            return left.Equals(right);
        }

        public static bool operator !=(SmallUri left, SmallUri right)
        {
            return !left.Equals(right);
        }
    }
}