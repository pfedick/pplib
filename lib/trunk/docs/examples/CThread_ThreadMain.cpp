#include <ppl7.h>

class MyClass : public ppl7::Thread
{
	public:
		virtual void threadMain();
};
 
void MyClass::threadMain()
{
	printf ("Client-Thread wurde gestartet.\n");
	while (1) {
		if(threadShouldStop()) break;	// Soll der Thread gestoppt werden?
		ppl7::MSleep(100);				// 100 Millisekunden warten
	}
	printf ("Client-Thread wurde beendet.\n");
}

int main(int argc, char **argv)
{
	MyClass Job;
	printf ("Der Haupthread startet einen Client-Thread...\n");
	Job.threadStart();
	printf ("Der Haupthread wartet nun 5 Sekunden...\n");
	ppl7::SSleep(5);
	printf ("Der Haupthread signalisiert dem Client-Thread, dass er sich beenden soll...\n");
	Job.threadSignalStop();
	printf ("Der Haupthread wartet, bis der Client-Thread beendet ist...\n");
	Job.threadStop();
	printf ("Der Client-Thread wurde erfolgreich gestoppt.\n");
	return 0;
}
