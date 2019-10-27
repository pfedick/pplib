
int DB_Example1() {
	ppl6::CAssocArray param;
	param.Set("type","mysql");
	param.Set("host","db.pfp.de");
	param.Set("port","3306");
	param.Set("user","patrick");
	param.Set("password","xxxxxxx");
	param.Set("dbname","test");

	ppl6::db::Database *db=ppl6::db::Connect(param);
	if (!db) {
		ppl6::PrintError();
		return 0;
	}
} // EOF

int DB_Example2() {
	ppl6::db::MySQL db;
	db.SetParam("host","db.pfp.de");
	db.SetParam("port","3306");
	db.SetParam("user","patrick");
	db.SetParam("password","xxxxxxx");
	db.SetParam("dbname","test");

	if (!db.Connect()) {
		ppl6::PrintError();
		return 0;
	}
} // EOF


int DB_Example3() {
	ppl6::CAssocArray param;
	param.Set("host","db.pfp.de");
	param.Set("port","3306");
	param.Set("user","patrick");
	param.Set("password","xxxxxxx");
	param.Set("dbname","test");
	ppl6::db::MySQL db;
	if (!db.Connect(param)) {
		ppl6::PrintError();
		return 0;
	}
} // EOF

int DB_Example4() {
	ppl6::CAssocArray param;
	param.Set("host","db.pfp.de");
	param.Set("port","3306");
	param.Set("user","patrick");
	param.Set("password","xxxxxxx");
	param.Set("dbname","test");
	ppl6::db::MySQL db;
	if (!db.ConnectCreate(param)) {
		ppl6::PrintError();
		return 0;
	}
} // EOF

int DB_MySQL_Example1() {
	// Verbindungsparameter festlegen
	ppl6::CAssocArray param;
	param.Set("host","db.pfp.de");
	param.Set("port","3306");
	param.Set("user","patrick");
	param.Set("password","xxxxxxx");
	param.Set("dbname","test");

	// Datenbank-Klasse anlegen
	ppl6::db::MySQL db;

	// Verbindung aufbauen
	if (!db.Connect(param)) {
		ppl6::PrintError();
		return 0;
	}

	// Query abschicken
	ppl6::db::Result *res=db.Query("select * from user oder by nachname, vorname");
	if (!res) {	// Fehler abfangen
		ppl6::PrintError();
	} else {
		printf ("Es wurden %i Datensätze gefunden\n",res->Rows());
		// Result wieder freigeben
		delete res;
	}

	// Die Verbindung wird durch den Destruktor der Klasse automatisch
	// ordnungsgemäß getrennt
} // EOF

int DB_Sybase_Example1() {
	// Verbindungsparameter festlegen
	ppl6::CAssocArray param;
	param.Set("host","db.pfp.de");
	param.Set("port","4711");
	param.Set("user","patrick");
	param.Set("password","xxxxxxx");
	param.Set("dbname","test");

	// Datenbank-Klasse anlegen
	ppl6::db::Sybase db;

	// Verbindung aufbauen
	if (!db.Connect(param)) {
		ppl6::PrintError();
		return 0;
	}

	// Query abschicken
	ppl6::db::Result *res=db.Query("select * from user oder by nachname, vorname");
	if (!res) {	// Fehler abfangen
		ppl6::PrintError();
	} else {
		printf ("Es wurden %i Datensätze gefunden\n",res->Rows());
		// Result wieder freigeben
		delete res;
	}

	// Die Verbindung wird durch den Destruktor der Klasse automatisch
	// ordnungsgemäß getrennt
} // EOF

int DB_Sybase_Example2() {
	// Verbindungsparameter festlegen
	ppl6::CAssocArray param;
	param.Set("host","db.pfp.de");
	param.Set("port","4711");
	param.Set("user","patrick");
	param.Set("password","xxxxxxx");
	param.Set("dbname","test");

	// Datenbank-Klasse anlegen
	ppl6::db::Sybase db;

	// Verbindung aufbauen
	if (!db.Connect(param)) {
		ppl6::PrintError();
		return 0;
	}

	// Query abschicken
	ppl6::db::Result *res=db.Query("select * from user oder by nachname, vorname");
	if (!res) {	// Fehler abfangen
		ppl6::PrintError();
	} else {
		int64_t rows=0;
		ppl6::CAssocArray row;
		// Durch das Ergebnis durchiterieren
		while (res->FetchArray(row)) {
			// Felder der aktuellen Ergebnis-Zeile befinden sich nun im Array "row" und
			// können verarbeitet werden
			printf ("%s %s\n",row["nachname"],row["vorname"]);
			rows++;
		}
		delete res;
	}

	// Die Verbindung wird durch den Destruktor der Klasse automatisch
	// ordnungsgemäß getrennt
} // EOF

int DB_Postgres_Example1() {
	// Verbindungsparameter festlegen
	ppl6::CAssocArray param;
	param.Set("host","db.pfp.de");
	param.Set("port","5432");
	param.Set("user","patrick");
	param.Set("password","xxxxxxx");
	param.Set("dbname","test");

	// Datenbank-Klasse anlegen
	ppl6::db::Postgres db;

	// Verbindung aufbauen
	if (!db.Connect(param)) {
		ppl6::PrintError();
		return 0;
	}

	// Query abschicken
	ppl6::db::Result *res=db.Query("select * from user oder by nachname, vorname");
	if (!res) {	// Fehler abfangen
		ppl6::PrintError();
	} else {
		printf ("Es wurden %i Datensätze gefunden\n",res->Rows());
		// Result wieder freigeben
		delete res;
	}

	// Die Verbindung wird durch den Destruktor der Klasse automatisch
	// ordnungsgemäß getrennt
} // EOF


int DB_Result_Example1() {
	// Zunächst erstellen wir eine Verbindung zu einer MySQL-Datenbank
	ppl6::CAssocArray param;
	param.Set("host","db.pfp.de");
	param.Set("port","3306");
	param.Set("user","patrick");
	param.Set("password","xxxxxxx");
	param.Set("dbname","test");
	ppl6::db::MySQL db;
	// Verbindung zur Datenbank aufbauen
	if (!db.Connect(param)) {
		ppl6::PrintError();
		return 0;
	}
	// Select abschicken
	ppl6::db::Result *res=db.Query("select * from user order by name");
	if (!res) {
		ppl6::PrintError();
		return 0;
	}
	printf ("Es wurden %lli Datensätze gefunden:\n",res->Rows());
	ppl6::CAssocArray row;
	// Wir verwenden Result::FetchArray, um durch das Ergebnis zu traversieren
	while ((res->FetchArray(row))) {
		printf ("Name: %s, Email: %s\n",row["name"],row["email"]);
	}
	delete res;	// Ergebnis löschen, Speicher freigeben
	db.Disconnect();
} // EOF


int DB_Save_Example1(ppl6::db::Database *db) {
	// Die Funktion geht davon aus, dass "db" eine gültige Datenbank-Verbindung
	// enthält.
	ppl6::CAssocArray Data;
	Data.Set("vorname","Patrick");
	Data.Set("nachname","Fedick");
	Data.Set("email","xxx@xxxx.xx");
	return db->Save("update","user",Data,"where userid=1");
} // EOF

int DB_Pool_Example1() {
	// Connect-Parameter festlegen
	ppl6::CAssocArray param;
	param.Set("type","sybase");
	param.Set("host","database.example.com");
	param.Set("port","1234");
	param.Set("user","demo");
	param.Set("password","demo");
	param.Set("dbname","test");

	// Pool erzeugen
	ppl6::db::Pool Pool;

	// Pool mit den Connect-Parametern initialisieren
	if (!Pool.SetConnectParams(param)) {
		ppl6::PrintError();
		return;
	}

	// Verbindung aus dem Pool holen
	ppl6::db::Database *db=Pool.Get();
	if (!db) {
		ppl6::PrintError();
		return;
	}

	// Verbindung verwenden
	...

	// Verbindung an den Pool zurückgeben
	if (!Pool.Release(db)) {
		ppl6::PrintError();
		return;
	}
} // EOF

int DB_PoolEx_Example1() {
	// Pool erzeugen
	ppl6::db::PoolEx Pool;

	// Connect-Parameter für die erste Datenbank festlegen
	ppl6::CAssocArray param1;
	param.Set("type","sybase");
	param.Set("host","database.example.com");
	param.Set("port","1234");
	param.Set("user","demo");
	param.Set("password","demo");
	param.Set("dbname","test");

	// Pool mit den Connect-Parametern anlegen
	// Die Datenbank bekommt die ID 1 und den Namen "Hauptdatenbank"
	if (!Pool.CreatePool(1,"Hauptdatenbank",param)) {
		ppl6::PrintError();
		return;
	}

	// Connect-Parameter für die zweite Datenbank festlegen
	ppl6::CAssocArray param1;
	param.Set("type","mysql");
	param.Set("host","mysql.example.com");
	param.Set("port","4711");
	param.Set("user","demo");
	param.Set("password","demo");
	param.Set("dbname","test");

	// Pool mit den Connect-Parametern anlegen.
	// Die Datenbank bekommt die ID 2 und den Namen "MySQL_1"
	if (!Pool.CreatePool(2,"DB2",param)) {
		ppl6::PrintError();
		return;
	}


	// Verbindung zur ersten Datenbank anhand des Namens holen
	ppl6::db::Database *db=Pool.Get("Hauptdatenbank");
	if (!db) {
		ppl6::PrintError();
		return;
	}

	// Verbindung verwenden
	...

	// Verbindung an den Pool zurückgeben
	if (!Pool.Release(db)) {
		ppl6::PrintError();
		return;
	}

	// Verbindung zur zweiten Datenbank anhand der ID holen
	ppl6::db::Database *db=Pool.Get(2);
	if (!db) {
		ppl6::PrintError();
		return;
	}

	// Verbindung verwenden
	...

	// Verbindung an den Pool zurückgeben
	if (!Pool.Release(db)) {
		ppl6::PrintError();
		return;
	}
} // EOF


ppl6::db::Result *DB_GenericResult_Example1(const CString &Query) {
	int num_fields;		// Nimmt die Anzahl Felder im Result auf
	// Datenbank-spezifische Code, der das Query ausführt
	// ...
	// Result-Objekt erstellen
	ppl6::db::GenericResult *res=new GenericResult;
	if (!res) {		// Out of Memory anfangen
		SetError(2);
		return NULL;
	}
	// Anzahl Felder im Result übergeben
	res->SetNumFields(num_fields);
	for (int i=0;i<num_fields;i++) {
		const char *field_name = xxxxx;	// Name des Feldes
		Result::Type field_type = xxxxx; // Datentyp
		// Namen und Feldtypen übergeben
		res->SetFieldName(i, field_name, field_type);
	}

	// Ergebniszeilen übergeben
	for (int64_t r=0;r<num_rows;r++) {
		// Neue Zeile
		res->NewRow();
		// Felder durchgehen
		for (int i=0;i<num_fields;i++) {
			void *value = xxxx;	// Pointer auf den Inhalt des Feldes
			int len_value = xxxx; // Anzahl Bytes des Feldes
			res->StoreField(i,value,len_value);
		}
	}
	// Betroffene Zeilen übergeben
	res->SetAffectedRows(num_rows);
	// Index aufbauen
	res->BuildIndex();
	// fertig
	return (ppl6::db::Result *)res;
} // EOF



