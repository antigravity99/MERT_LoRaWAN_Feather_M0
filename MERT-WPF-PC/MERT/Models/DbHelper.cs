using System;
using System.Collections.Generic;
using System.Collections.ObjectModel;
using System.Data;
using System.Data.SqlClient;
using System.Diagnostics;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace MERT
{
    class DbHelper
    {
        const string CONNECTION_STRING = "Data Source=(LocalDB)\\MERT;Database=MERTObjectsModel;Integrated Security=true;";

        const string C_ADDRESS = "Address";
        const string C_LAST_READING_DATE_TIME = "Last_Reading_Date_Time";
        const string C_ID = "Id";
        const string C_READING_TYPE = "Reading_Type";
        const string C_READING = "Reading";
        const string C_VIB_SAMPLE_RATE = "Vib_Sample_Rate";
        const string C_DATE_TIME = "Date_Time";
        const string C_LAST_READING_SECONDS = "Last_Reading_Seconds";
        const string C_TEMP_ID = "TempId";

        public DbHelper()
        {
            //using (SqlConnection connection = new SqlConnection())
            //{
            //    connection.ConnectionString = CONNECTION_STRING;
            //    try
            //    {

            //        // ANSI SQL way.  Works in PostgreSQL, MSSQL, MySQL.  
            //        string statement = "select case when exists((select * from information_schema.tables where table_name = 'Motes')) then 1 else 0 end";

            //        var cmd = new SqlCommand(statement);

            //        connection.Open();

            //        if(!((int)cmd.ExecuteScalar() == 1))
            //        {

            //        }

            //    }
            //    catch
            //    {


            //    }

            //    Debug.WriteLine("State: {0}", connection.State);
            //    Debug.WriteLine("ConnectionString: {0}",
            //        connection.ConnectionString);
            //}

            PopulateMotesTable();
        }

        public void PopulateMotesTable()
        {
            using (SqlConnection connection = new SqlConnection(CONNECTION_STRING))
            {
                for(int i = 0; i < 16; i++)
                {
                    if (AddressExistsInMotesTable(i))
                        continue;

                    string insert = "INSERT INTO MERTObjectsModel.dbo.Motes (Address, Last_Reading_Date_Time) VALUES (@Address, SYSDATETIME())";

                    using (SqlCommand command = new SqlCommand(insert, connection))
                    {
                        command.Parameters.AddWithValue("@Address", i);
                        if(connection.State == ConnectionState.Closed)
                            connection.Open();
                        int result = command.ExecuteNonQuery();

                        // Check Error
                        if (result < 0)
                            Debug.WriteLine("Error inserting data into Database!");
                        //connection.Close();
                    }
                }
            }
        }

        private bool AddressExistsInMotesTable(int address)
        {
            bool exists = false;
            using (SqlConnection connection = new SqlConnection(CONNECTION_STRING))
            {
                string statement = $"SELECT COUNT(*) FROM MERTObjectsModel.dbo.Motes AS M WHERE M.Address = {address}";

                using (SqlCommand command = new SqlCommand(statement, connection))
                {
                    if (connection.State == ConnectionState.Closed)
                        connection.Open();
                    int result = (int)command.ExecuteScalar();
                    if (result > 0)
                        exists = true;
                }
            }
            return exists;
        }

        private DataTable GetDataTable(string statement)
        {
            DataTable dt = new DataTable();
            try
            {
                using (SqlConnection connection = new SqlConnection(CONNECTION_STRING))
                {
                    using (SqlCommand command = new SqlCommand(statement, connection))
                    {
                        if (connection.State == ConnectionState.Closed)
                            connection.Open();
                        SqlDataAdapter dataAdapter = new SqlDataAdapter(command);
                        // this will query your database and return the result to your datatable
                        dataAdapter.Fill(dt);
                        connection.Close();
                        dataAdapter.Dispose();
                    }
                }
            }
            catch (Exception ex)
            {
                Debug.WriteLine("Error getting DataTable!");
                Debug.WriteLine(ex.Message);
                Debug.WriteLine(ex.StackTrace);
            }
            return dt;
        }

        public DataTable GetMoteTableAsDataTable()
        {
            string statement = "SELECT * FROM MERTObjectsModel.dbo.Motes";
            return GetDataTable(statement);
        }

        public DataTable GetMoteTableAsDataTable(string address)
        {
            string statement = $"SELECT * FROM MERTObjectsModel.dbo.Motes AS M WHERE M.Address = {address}";
            return GetDataTable(statement);
        }

        public ObservableCollection<ListViewModel> GetMotesbservableCollection()
        {
            ObservableCollection<ListViewModel> list = new ObservableCollection<ListViewModel>();
            
            string statement = $"SELECT M.Address, DATEDIFF(second, M.Last_Reading_Date_Time, SYSDATETIME()) AS Last_Reading_Seconds FROM MERTObjectsModel.dbo.Motes As M";

            DataTable dt = GetDataTable(statement);

            foreach (DataRow r in dt.Rows)
            {
                int seconds = int.Parse(r[C_LAST_READING_SECONDS].ToString());

                var item = new ListViewModel()
                {
                    MoteAddress = int.Parse(r[C_ADDRESS].ToString()),
                    MoteType = Values.DeviceTypes.Mote.ToString(),
                    IsActive = seconds < 15 ? true : false
                };

                list.Add(item);
            }
            return list;
        }

        public ObservableCollection<SensorReadingsModel> GetSensorReadingModelCollection(int address, string readingType, string minDateTime, string maxDateTime)
        {
            ObservableCollection<SensorReadingsModel> list = new ObservableCollection<SensorReadingsModel>();
            

            DataTable dt = GetSensorReadingDataTableFiltered( address, readingType, minDateTime, maxDateTime);

            foreach (DataRow r in dt.Rows)
            {
                double sampleRate = 0;
                double.TryParse(r[C_VIB_SAMPLE_RATE].ToString(), out sampleRate);
                var item = new SensorReadingsModel()
                {
                    TempId = r[C_TEMP_ID].ToString(),
                    Address = int.Parse(r[C_ADDRESS].ToString()),
                    Reading_Type = r[C_READING_TYPE].ToString(),
                    Reading = r[C_READING].ToString(),
                    Vib_Sample_Rate = sampleRate,
                    Date_Time = r[C_DATE_TIME].ToString()
                };

                list.Add(item);
            }
            return list;
        }

        public DataTable GetSensorReadingDataTableFiltered(int address, string readingType, string minDateTime, string maxDateTime)
        {
            string statement;

            if (minDateTime == null)
            {
                statement = "SELECT TOP(2000) ROW_NUMBER() OVER(ORDER BY Date_Time ASC) AS TempId, " +
                                "SR.Address, SR.Reading_Type, SR.Reading, SR.Vib_Sample_Rate, SR.Date_Time " +
                                "FROM MERTObjectsModel.dbo.SensorReading AS SR " +
                                $"WHERE SR.Address = {address} AND SR.Reading_Type LIKE '%{readingType}%' " +
                                "ORDER BY SR.Date_Time DESC";
            }
            else
            {
                statement = "SELECT TOP(2000) ROW_NUMBER() OVER(ORDER BY Date_Time ASC) AS TempId, " +
                                "SR.Address, SR.Reading_Type, SR.Reading, SR.Vib_Sample_Rate, SR.Date_Time " +
                                "FROM MERTObjectsModel.dbo.SensorReading AS SR " +
                                $"WHERE SR.Address = {address} AND SR.Reading_Type LIKE '%{readingType}%' " +
                                $"AND SR.Date_Time between '{minDateTime}' and '{maxDateTime}' " +
                                "ORDER BY SR.Date_Time DESC";
            }
            return GetDataTable(statement);
        }

        public DataTable GetSensorReadingDataTable()
        {
            
            string statement = "SELECT * FROM MERTObjectsModel.dbo.SensorReading";
            
            return GetDataTable(statement);            
        }



        public void InsertReading(Request req, double? sampleRate)
        {
            using (SqlConnection connection = new SqlConnection(CONNECTION_STRING))
            {
                string insertSensor = "INSERT INTO MERTObjectsModel.dbo.SensorReading (Address, Reading_Type, Reading, Vib_Sample_Rate, Date_Time) VALUES (@Address, @Reading_Type, @Reading, @Vib_Sample_Rate, SYSDATETIME())";
                using (SqlCommand command = new SqlCommand(insertSensor, connection))
                {
                    sampleRate = sampleRate == null ? 0.0 : sampleRate; 
                    command.Parameters.AddWithValue("@Address", req.Add);
                    command.Parameters.AddWithValue("@Reading_Type", req.Key);
                    command.Parameters.AddWithValue("@Reading", req.Val);
                    command.Parameters.AddWithValue("@Vib_Sample_Rate", sampleRate);
                    if (connection.State == ConnectionState.Closed)
                        connection.Open();
                    int result = command.ExecuteNonQuery();

                    // Check Error
                    if (result < 0)
                        Debug.WriteLine("Error inserting data into Database!");
                    //connection.Close();
                }

                string updateMote = $"UPDATE MERTObjectsModel.dbo.Motes SET Last_Reading_Date_Time = SYSDATETIME() WHERE Address = {req.Add}";
                using (SqlCommand command = new SqlCommand(updateMote, connection))
                {
                    if (connection.State == ConnectionState.Closed)
                        connection.Open();
                    int result = command.ExecuteNonQuery();

                    // Check Error
                    if (result < 0)
                        Debug.WriteLine("Error inserting data into Database!");
                    //connection.Close();
                }
            }
        }
    }
}
