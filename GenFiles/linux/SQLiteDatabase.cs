using System;
using System.Collections.Generic;
using System.Data;
using System.Data.Common;
using Mono.Data.Sqlite;
using System.Diagnostics;

namespace GenFiles
{
	public class SQLiteDatabase : IDisposable
	{
		public string DbConnection { get { return _dbConnection; } }
		String _dbConnection;
		SqliteConnection _connection;
		SqliteCommand _generalCommand;
		SqliteCommand _dataTableCommand;
		private object _lock = new object();

		public SqliteConnection Connection { get { return _connection; } }

		public SqliteCommand GeneralCommand { get { return _generalCommand; } }
		public SqliteCommand DataTableCommand { get { return _dataTableCommand; } }

		/// <summary>
		///     Single Param Constructor for specifying the DB file.
		/// </summary>
		/// <param name="inputFile">The File containing the DB</param>
		public SQLiteDatabase(String connection)
		{
			_dbConnection = connection;
			_connection = new SqliteConnection(_dbConnection);
			try
			{
				_connection.Open();
			}
			catch (AccessViolationException ex)
			{
				GenLib.Log.LogService.LogException("Error while opening the connection: " + ex.Message, ex);
			}
			
			_generalCommand = new SqliteCommand(_connection);
			_dataTableCommand = new SqliteCommand(_connection);
		}

		public SqliteCommand CreateCommand(string command)
		{
			return new SqliteCommand(command, _connection);
		}

		public void DisposeCommand(SqliteCommand command)
		{
			//command.Connection.Dispose();
			command.Dispose();
		}

		/// <summary>
		///     Single Param Constructor for specifying advanced connection options.
		/// </summary>
		/// <param name="connectionOpts">A dictionary containing all desired options and their values</param>
		public SQLiteDatabase(Dictionary<String, String> connectionOpts)
		{
			String str = "";
			foreach (KeyValuePair<String, String> row in connectionOpts)
				str += String.Format("{0}={1}; ", row.Key, row.Value);
			str = str.Trim().Substring(0, str.Length - 1);
			_dbConnection = str;
			_connection = new SqliteConnection(_dbConnection);
			_connection.Open();
			_generalCommand = new SqliteCommand(_connection);
			_dataTableCommand = new SqliteCommand(_connection);
		}

		//~SqliteDatabase()
		//{
		//    CloseConnection();
		//}

		/// <summary>
		///     Allows the programmer to run a query against the Database.
		/// </summary>
		/// <param name="sql">The SQL to run</param>
		/// <returns>A DataTable containing the result set.</returns>
		public DataTable GetDataTable(string sql)
		{
			DataTable dt = new DataTable();
			try
			{
				lock (_lock)
				{
					if (!String.IsNullOrEmpty(sql))
						_dataTableCommand.CommandText = sql;

					SqliteDataAdapter adapter =new SqliteDataAdapter(_dataTableCommand);
					try
					{
						adapter.Fill(dt);
					}
					catch (System.Exception ex) 
					{
						GenLib.Log.LogService.LogException("Error while executing a query: " + sql, ex);
						foreach (DataRow row in dt.GetErrors())
							Trace.WriteLine(row.RowError);
					}
				}
			}
			catch (Exception ex) 
			{
				GenLib.Log.LogService.LogException("Error while executing a query: " + sql, ex);
				throw;
			}
			return dt;
		}


		/// <summary>
		///     Allows the programmer to run a query against the Database.
		/// </summary>
		/// <param name="sql">The SQL to run</param>
		/// <returns>A DataTable containing the result set.</returns>
		public DataTable GetDataTable(SqliteCommand command)
		{
			DataTable dt = new DataTable();
			try
			{
				lock (_lock)
				{
					SqliteDataAdapter adapter =new SqliteDataAdapter(command);
					try
					{
						adapter.Fill(dt);
					}
					catch (System.Exception ex) 
					{ 
						GenLib.Log.LogService.LogException("Error while executing a query: " + command.CommandText, ex);
						foreach (DataRow row in dt.GetErrors())
							Trace.WriteLine(row.RowError);
					}
				}
			}
			catch (Exception ex)
			{
				GenLib.Log.LogService.LogException("Error while executing a query: " + command.CommandText, ex);
				throw;
			}
			return dt;
		}

		/// <summary>
		///     Allows the programmer to interact with the database for purposes other than a query.
		/// </summary>
		/// <param name="sql">The SQL to be run.</param>
		/// <returns>An Integer containing the number of rows updated.</returns>
		public int ExecuteNonQuery(string sql)
		{
			try
			{
				lock (_lock)
				{

					if (!String.IsNullOrEmpty(sql))
						_generalCommand.CommandText = sql;
					return _generalCommand.ExecuteNonQuery();		// return the number of updated rows
				}
			}
			catch (Exception ex)
			{
				GenLib.Log.LogService.LogException("Error while executing a query: " + sql, ex);
				throw;
				//return -1;
			}
		}

		/// <summary>
		///     Allows the programmer to interact with the database for purposes other than a query.
		/// </summary>
		/// <param name="sql">The SQL to be run.</param>
		/// <returns>An Integer containing the number of rows updated.</returns>
		public int ExecuteNonQuery(SqliteCommand command)
		{
			try
			{
				lock (_lock)
				{
					return command.ExecuteNonQuery();		// return the number of updated rows
				}
			}
			catch (Exception ex)
			{
				GenLib.Log.LogService.LogException("Error while executing a query: " + command.CommandText, ex);
				throw;
				//return -1;
			}
		}

		/// <summary>
		///     Allows the programmer to retrieve single items from the DB.
		/// </summary>
		/// <param name="sql">The query to run.</param>
		/// <returns>A string.</returns>
		public object ExecuteScalar(string sql)
		{
			try
			{
				lock (_lock)
				{
					if (!String.IsNullOrEmpty(sql))
						_generalCommand.CommandText = sql;
					return _generalCommand.ExecuteScalar();
				}
			}
			catch (Exception ex)
			{
				GenLib.Log.LogService.LogException("Error while executing a query: " + sql, ex);
				throw;
				//return null;
			}
		}

		/// <summary>
		///     Allows the programmer to retrieve single items from the DB.
		/// </summary>
		/// <param name="sql">The query to run.</param>
		/// <returns>A string.</returns>
		public object ExecuteScalar(SqliteCommand command)
		{
			try
			{
				lock (_lock)
				{
					return command.ExecuteScalar();
				}
			}
			catch (Exception ex)
			{
				GenLib.Log.LogService.LogException("Error while executing a query: " + command.CommandText, ex);
				throw;
				//return null;
			}
		}

		/// <summary>
		///     Allows the programmer to easily delete all data from the DB.
		/// </summary>
		/// <returns>A boolean true or false to signify success or failure.</returns>
		public bool ClearDB()
		{
			DataTable tables;
			try
			{
				tables = this.GetDataTable("select NAME from SQLITE_MASTER where type='table' order by NAME;");
				foreach (DataRow table in tables.Rows)
				{
					this.ClearTable(table["NAME"].ToString());
				}
				return true;
			}
			catch
			{
				return false;
			}
		}

		/// <summary>
		///     Allows the user to easily clear all data from a specific table.
		/// </summary>
		/// <param name="table">The name of the table to clear.</param>
		/// <returns>A boolean true or false to signify success or failure.</returns>
		public bool ClearTable(String table)
		{
			try
			{
				this.ExecuteNonQuery(String.Format("delete from {0};", table));
				return true;
			}
			catch (Exception ex)
			{
				GenLib.Log.LogService.LogException("Exception while calling ClearTable: ", ex);
				return false;
			}
		}

		/// <summary>
		/// Get the columns of the table and their type
		/// </summary>
		/// <param name="tableName"></param>
		/// <returns>["column 0", "type 0", "column 1", "type 1", ...]</returns>
		public List<string> GetTableColumns(string tableName)
		{
			try
			{
				DataTable table = GetDataTable(String.Format("PRAGMA table_info('{0}')", tableName));
				List<string> columnData = new List<string>();
				foreach (DataRow row in table.Rows)
				{
					columnData.Add((string)row[1]);
					columnData.Add((string)row[2]);
				}
				return columnData;
			}
			catch (Exception ex)
			{
				GenLib.Log.LogService.LogException("Error while GetTableColumns: ", ex);
				return new List<string>();
			}
			
		}

		/// <summary>
		/// Check if the given table corresponds to the data in the columnData table
		/// </summary>
		/// <param name="tableName">name of the table to check</param>
		/// <param name="columnData">names and types of the columns that we expect to find</param>
		/// <returns>true if the table matches the given columnData, false otherwise</returns>
		public bool CheckTableColumns(string tableName, List<string> columnData)
		{
			List<string> existingColumnData = GetTableColumns(tableName);
			if (existingColumnData.Count != columnData.Count) return false;
			for (int i = 0; i < columnData.Count; i++)
			{
				if (existingColumnData[i] != columnData[i])
					return false;
			}
			return true;
		}

		public SqliteTransaction BeginTransaction()
		{
			return Connection.BeginTransaction();
		}

		#region IDisposable Members
		private bool _disposed;
		public void Dispose()
		{
			if (_disposed) return;
			if (_generalCommand != null)
				_generalCommand.Dispose();
			_generalCommand = null;
			if (_dataTableCommand != null)
				_dataTableCommand.Dispose();
			_dataTableCommand = null;
			if (_connection != null)
			{
				_connection.Close();
				_connection.Dispose();
			}
			_connection = null;
			_disposed = true;
		}


		#endregion
	}
}