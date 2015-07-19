#include <ppl6.h>

class MyItem : public ppl6::CTreeItem
{
	public:
		MyItem() {v=0;}
		MyItem(int value) {v=value;}

		virtual int CompareNode(CTreeItem *item);
        virtual int CompareValue(void *value);

		int v;
};

int MyItem::CompareNode(CTreeItem *item)
{
	MyItem *other=(MyItem*)item;
	if (other->v > v) return 1;
	if (other->v < v) return -1;
	return 0;
}

int MyTree::CompareValue(void *value)
{
	int *other = (int*)value;
	if (*other > v) return 1;
	if (*other < v) return -1;
	return 0;
}

int main(int argc, char **argv)
{

	ppl6::CTree Tree;

	MyItem *item, *ii;

	item=new MyItem(20);
	Tree.Add(item);
	item=new MyItem(15);
	Tree.Add(item);
	item=new MyItem(30);
	Tree.Add(item);
	item=new MyItem(4);
	Tree.Add(item);
	item=new MyItem(8);
	Tree.Add(item);
	item=new MyItem(33);
	Tree.Add(item);

	Tree.Reset();
	while ((ii=(MyItem*)Tree.GetNext())) {
		printf ("%i\n",ii->v);
	}
	return 0;
}

