int Socket_Example1() {
	ppl6::CTCPSocket Socket;
	// Wir bauen eine Verbindung zum Zielrechner auf
	if (!Socket.Connect("localhost",9020)) {
		ppl6::PrintError();
		return 0;
	}
	ppl6::CString Daten="Mein erstes Datenpaket";
	// Nun senden wir das Datenpaket
	if (!Socket.Write(Daten)) {
		ppl6::PrintError();
		return 0;
	}
	// Und warten auf die Antwort
	if (!Socket.ReadOnce(Daten,100)) {
		ppl6::PrintError();
		return 0;
	}
	printf ("Daten erhalten: %s",(const char *)Daten);
	// Verbindung trennen
	Socket.Disconnect();
} // EOF

// Socket_Example2

// Zunächst leitet wir eine eigene Klasse von CTCPSocket ab, in der wir
// nur die virtuelle Funktion ReceiveConnect implementieren:
class MySocket : public ppl6::CTCPSocket
{
	public:
		virtual int ReceiveConnect(CTCPSocket *socket, const char *host, int port);
};

// Implementation der Funktion
int MySocket::ReceiveConnect(CTCPSocket *socket, const char *host, int port)
{
	// Speicher für die eingehenden Datenpakete allokieren
	char *buffer=(char*)malloc(100);
	// Wir schalten den Socket auf Non-Blocking
	socket->SetBlocking(false);
	int bytes;
	// Endlosschleife, die erst bei Auftreten eines Fehlers beendet wird, z.B.
	// infolge einer Verbindungstrennung
	while (1) {
		// Auf Daten warten, maximal aber 100 Millisekunden
		if (socket->WaitForIncomingData(0,10000)) {
			// Daten lesen
			if (!socket->ReadOnce(buffer,100)) {
				// Es ist ein Fehler aufgetreten
				ppl6::PrintError();
				break;
			}
			// Wieviel Bytes haben wir bekommen?
			bytes=socket->GetBytesRead();
			// Datenpaket zurückschicken
			if (!socket->Write(buffer,bytes)) {
				// Es ist ein Fehler aufgetreten
				ppl6::PrintError();
				break;
			}
		}
	}
	free(buffer);
	return 0;
}

// Nun die Main-Funktion
int main (int argc, char **argv)
{
	// Instanz von MySocket definieren
	MySocket Socket;
	printf ("Starte Server auf localhost:9020\n");
	// Server auf "localhost" und Port 9020 binden
	if (!Socket.Bind("localhost",9020)) {
		ppl6::PrintError();
		return 0;
	}
	// Auf Verbindungen warten
	if (!Socket.Listen()) {
		ppl6::PrintError();
		return 0;
	}
} // EOF

// Socket_Example3

// Zunächst leitet wir eine eigene Klasse von CTCPSocket ab, in der wir
// nur die virtuelle Funktion ReceiveConnect implementieren:
class MySocket : public ppl6::CTCPSocket
{
	public:
		// Eine SSL-Klasse instanziieren
		ppl6::CSSL ssl;

		virtual int ReceiveConnect(CTCPSocket *socket, const char *host, int port);
};

// Implementation der Funktion
int MySocket::ReceiveConnect(CTCPSocket *socket, const char *host, int port)
{
	// SSL für diese Verbindung initialisieren
	if (!socket->SSL_Init(&ssl)) {
		ppl6::PrintError();
		return 0;
	}
	// Und starten
	if (!socket->SSL_Init_Server()) {
		ppl6::PrintError();
		return 0;
	}
	// Wir schalten den Socket auf Non-Blocking
	socket->SetBlocking(false);

	// Auf SSL-Handshake warten
	if (!socket->SSL_WaitForAccept(10)) {			// 10-Sekunden Timeout
		ppl6::PrintError();
		socket->SetBlocking(true);
		return 0;
	}
	// Ab hier haben wir eine verschlüsselte Verbindung

	// Speicher für die eingehenden Datenpakete allokieren
	char *buffer=(char*)malloc(100);
	int bytes;
	// Endlosschleife, die erst bei Auftreten eines Fehlers beendet wird, z.B.
	// infolge einer Verbindungstrennung
	while (1) {
		// Auf Daten warten, maximal aber 100 Millisekunden
		if (socket->WaitForIncomingData(0,10000)) {
			// Daten lesen
			if (!socket->ReadOnce(buffer,100)) {
				// Es ist ein Fehler aufgetreten
				ppl6::PrintError();
				break;
			}
			// Wieviel Bytes haben wir bekommen?
			bytes=socket->GetBytesRead();
			// Datenpaket zurückschicken
			if (!socket->Write(buffer,bytes)) {
				// Es ist ein Fehler aufgetreten
				ppl6::PrintError();
				break;
			}
		}
	}
	free(buffer);
	return 0;
}

int main(int argc, char **argv)
{
	ppl6::Cppl6Core *core=ppl6::PPLInit();
	if (!ppl6::SSL_Init()) {						// OpenSSL initialisieren
		ppl6::PrintError();
		return 0;
	}

	// Instanz von MySocket definieren
	MySocket Socket;

	// SSL-Klasse darin mit SSLv3 als Methode initialisieren
	if (!Socket.ssl.Init(ppl6::CSSL::SSLv3)) {
		ppl6::PrintError();
		return 0;
	}
	// Zertifikat für den Server laden
	if (!Socket.ssl.LoadCertificate("mein_zertifikat.pem","mein_schluessel.key","MeinPassword")) {
		ppl6::PrintError();
		return 0;
	}


	printf ("Starte Server auf localhost:9020\n");
	// Server auf "localhost" und Port 9020 binden
	if (!Socket.Bind("localhost",9020)) {
		ppl6::PrintError();
		return 0;
	}
	// Auf Verbindungen warten
	if (!Socket.Listen()) {
		ppl6::PrintError();
		return 0;
	}
}



// EOF
