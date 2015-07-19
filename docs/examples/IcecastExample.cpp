#include <ppl7.h>
#include <ppl7-audio.h>


int main(int argc, char **argv)
{
	ppl7::Icecast ice;	// Icecast-Objekt anlegen
	ppl7::File file;

	// Version der libshout ausgeben
	printf ("libshout-Version: %s\n",(const char*)ice.version());

	// Zu streamende MP3-Datei öffnen
	if (!file.Open("/musik/powerplay/Ausgaben/Powerplay 152.mp3")) {
		ppl6::PrintError();
		return 1;
	}

	// Buffer für Audio-Daten reservieren
	char *buffer=(char*)malloc(8192);
	if (!buffer) {
		printf("Kein Speicher!\n");
		return 1;
	}

	// Exceptions abfangen
	try {
		// Stream-Format auf MP3 setzen
		ice.setFormatMP3();

		// Verbindungsparameter definieren
		ice.setConnection("icecast.pfp.de",8000,"icesource");

		// Mount-Punkt festlegen
		ice.setMount("/live");

		// Stream-Informationen setzen
		ice.setGenre("Trance House Techno Electro");
		ice.setName("Patrick F.'s Powerplay");
		ice.setUrl("http://powerplay.pfp.de/");
		ice.setDescription("Trance, House, Techno, Electro, mixed by Patrick Fedick");

		// Verbindung zum Server aufbauen
		ice.connect();

		// Name des aktuellen Titels als Metainformation senden
		ice.setTitle("Powerplay Ausgabe 152");

		printf ("Beginne Stream\n");
		size_t bytes;
		// Quelldatei Schrittweise einlesen
		while ((bytes=file.Fread(buffer,1,8192))) {
			// Mit Server synchronisieren
			ice.sync();
			// Daten senden
			ice.send(buffer,bytes);
		}
		printf ("Stream beendet\n");

	} catch (ppl6::Exception& e) {
		// Ursache der Exception ausgeben
		printf ("Exception aufgetreten: %s\n",e.what());
		free(buffer);
		return 1;
	}
	free(buffer);
	return 0;
}

