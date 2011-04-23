#include <stdio.h>
#include <string.h>
#include <ppl6.h>


class MyApp : public ppl6::CApplication
{
	public:
		int Main();
};

PPL6APP(MyApp);


int MyApp::Main()
{
	bool bHaveHelpParam=HaveArg("-h");
	const char *file=GetArg("-f");

	if (bHaveHelpParam) {
		printf ("You need to add Parameter -f followed by a filename\n");
		return 0;
	}
	if (file==NULL || strlen(file)==0) {
		printf ("no filename\n");
		return 1;
	}
	printf ("Filename: %s\n",file);
	return 0;
}
