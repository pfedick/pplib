#include <ppl6.h>

class MyClass : public ppl6::CpplThread
{
	public:
		virtual void ThreadMain(void *param);
};
 
void MyClass::ThreadMain(void *param)
{
	printf ("Client-Thread wurde gestartet.\n");
	while (1) {
		if(ThreadShouldStop()) break;	// Soll der Thread gestoppt werden?
		ppl6::MSleep(100);				// 100 Millisekunden warten
	}
	printf ("Client-Thread wurde beendet.\n");
}

int main(int argc, char **argv)
{
	MyClass Job;
	printf ("Der Haupthread startet einen Client-Thread...\n");
	Job.ThreadStart();
	printf ("Der Haupthread wartet nun 5 Sekunden...\n");
	ppl6::SSleep(5);
	printf ("Der Haupthread signalisiert dem Client-Thread, dass er sich beenden soll...\n");
	Job.ThreadSignalStop();
	printf ("Der Haupthread wartet, bis der Client-Thread beendet ist...\n");
	Job.ThreadStop();
	printf ("Der Client-Thread wurde erfolgreich gestoppt.\n");
	return 0;
}
