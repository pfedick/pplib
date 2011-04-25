/*******************************************************************************
 * This file is part of "Patrick's Programming Library", Version 7 (PPL7).
 * Web: http://www.pfp.de/ppl/
 *
 * $Author: pafe $
 * $Revision: 32 $
 * $Date: 2011-04-24 20:16:39 +0000 (So, 24 Apr 2011) $
 * $Id: Array.cpp 32 2011-04-24 20:16:39Z pafe $
 * $URL: https://pplib.svn.sourceforge.net/svnroot/pplib/lib/trunk/src/types/Array.cpp $
 *
 *******************************************************************************
 * Copyright (c) 2011, Patrick Fedick <patrick@pfp.de>
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *     * Redistributions of source code must retain the above copyright
 *       notice, this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above copyright
 *       notice, this list of conditions and the following disclaimer in the
 *       documentation and/or other materials provided with the distribution.
 *     * Neither the name of the copyright holder nor the names of its
 *       contributors may be used to endorse or promote products derived
 *       from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDER AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER AND CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF
 * THE POSSIBILITY OF SUCH DAMAGE.
 *******************************************************************************/

#include "prolog.h"
#ifdef HAVE_STDIO_H
#include <stdio.h>
#endif
#ifdef HAVE_STDLIB_H
#include <stdlib.h>
#endif
#ifdef HAVE_STRING_H
#include <string.h>
#endif
#ifdef HAVE_STDARG_H
#include <stdarg.h>
#endif

#include "ppl7.h"


namespace ppl7 {


/*!\class AVLTreeController
 * \ingroup PPLGroupDataTypes
 * \brief Virtuelle Basisklasse für AVLTrees
 *
 * \desc
 * Dies ist eine rein virtuelle Basisklasse, die von AVLTree reimplementiert wird.
 */

/*!\fn AVLTreeController::compare
 * \brief Zwei Elemente des Baums vergleichen
 *
 * \desc
 * Damit Elemente sortiert in den Baum eingehangen werden können, muss eine Möglichkeit
 * bestehen zwei Elemente zu vergleichen. Dies wird mit dieser Methode realisiert.
 * Da jeder Baum andere Daten enthalten kann, muss die Methode für jeden Datentyp
 * reimplementiert werden.
 *
 * \param[in] value1 Pointer auf das erste Element. Der Pointer zeigt direkt auf die
 * Daten des Knotens, nicht auf die Verwaltungsdaten des Knotens.
 *
 * \param[in] value2 Pointer auf das zweite Element. Der Pointer zeigt direkt auf die
 * Daten des Knotens, nicht auf die Verwaltungsdaten des Knotens.
 *
 * \return Die Funktion muss einen der folgenden 4 Werte zurückliefern:
 * - 0: Ist der Wert in \p value2 identisch mit \p value1, muss 0 zurückgegeben werden.
 * - +1: Ist der Wert in \p value2 größer als der Wert in \p value1, muss +1 zurückgegeben werden
 * - -1: Ist der Wert in \p value2 kleiner als der Wert in \p value1, muss -1 zurückgegeben werden
 * \exception AVLTree::NoTreeControllerException: Wird geworfen, wenn keine Vergleichsoperatoren
 * implementiert wurden (kein AVLTreeController).
 *
 * \attention Beim Vergleich zweier Strings kann die Funktion \c strcmp nicht direkt
 * verwendet werden, da sie laut Definition Werte kleiner oder größer 0 liefert, aber
 * nicht exakt -1 oder +1.
 *
 * \example
 * Beispiel für eine Implementierung:
 \code
class MyItem
{
	public:
		MyItem(const char *name) {
			Name=name;
		}
		ppl6::CString Name;
};

class MyTree : public ppl6::AVLTree
{
	public:
		~MyTree() {
			Clear();
		}
		virtual int	Compare(const void *value1, const void *value2) {
			MyItem *i1=(MyItem*)value1;
			MyItem *i2=(MyItem*)value2;
			int ret=i2->Name.StrCmp(i1->Name);
			if (ret<0) return -1;
			if (ret>0) return 1;
			return 0;
		}
}
\endcode
*/


/*!\fn AVLTreeController::getValue
 * \brief Daten als String ausgeben
 *
 * Diese Funktion wird durch AVLTree::PrintNodes aufgerufen, um den Inhalt des Baums
 * anzeigen zu können. Da jeder Baum andere Daten enthalten kann,
 * muss die Methode für jeden Datentyp reimplementiert werden. Sie bekommt als Eingabe
 * einen Pointer \p item auf die Daten des Knotens und einen String \p buffer, in dem
 * eine textuelle Darstellung des Dateninhalts abgelegt werden soll.
 * \par
 * Die Implementierung der Funktion ist optional.
 *
 * @param[in] item Pointer auf die Daten des Knotens
 * @param[out] buffer String, in dem die textuelle Darstellung der Daten erfolgen soll.
 * @return Bei Erfolg soll die Funktion 1 zurückgeben, im Fehlerfall 0. Wird 0 zurückgegeben,
 * wird der Knoten nicht ausgegeben.
 */


/*!\fn AVLTreeController::destroyValue
 * \brief Datenelement löschen
 *
 * \desc
 * Diese Funktion wird aufgerufen, wenn ein Wert aus dem Baum gelöscht werden soll,
 * bzw. wenn der komplette Baum gelöscht wird. Da jeder Baum andere Daten enthalten kann,
 * muss die Methode für jeden Datentyp reimplementiert werden.
 *
 * @param item Pointer auf die zu löschenden Daten. Der Pointer zeigt direkt auf die
 * Daten des Knotens, nicht auf die Verwaltungsdaten des Knotens.
 * @return Die Funktion sollte 1 bei erfolgreichem Löschen zurückgeben, im Fehlerfall 0.
 * Allerdings wird der Rückgabewert gegenwärtig nicht geprüft, so dass gegebenenfalls
 * unbemerkt ein Memory-Leak entstehen kann.
 */


/*!\class AVLTree
 * \brief AVL-Bäume mit nichtypisierten Elementen
 * \ingroup PPLGroupDataTypes
 *
 * \desc
 * Diese Klasse kann zur Verwaltung beliebiger Elemente in einem sortierten AVL-Baum
 * verwendet werden. Die Elemente sind dabei beliebig und werden nur durch ihren
 * Pointer referenziert. Im Gegensatz zu CTree werden die zur Baumstruktur benötigten Daten
 * von der Klasse selbst verwaltet und sind nicht Bestandteil der einzelnen Elemente.
 * \par
 * Dafür kann die Klasse aber nicht direkt verwendet werden, es muss zunächst eine
 * Ableitung erstellt werden, in der mindestens die virtuelle Funktion AVLTree::Compare
 * implementiert werden muss. Optional können auch noch AVLTree::DestroyValue und
 * AVLTree::GetValue implementiert werden.
 * \par
 * Als Alternative kann auch eine Klasse von CTreeController abgeleitet werden, in der die
 * genannten Funktionen implementiert werden. Diese wird über AVLTree::SetTreeController
 * der AVL-Klasse mitgeteilt, wodurch fortan deren Methoden verwendet werden.
 *
 * \attention Die Klasse verwaltet keinen eigenen Mutex und ist somit nicht Thread-sicher.
 * Die Aufrufende Anwendung muss selbst sicherstellen, dass die Klasse nicht gleichzeitig
 * von mehreren Threads verwendet wird.
 *
 */

/*!\var AVLTree::root
 * \brief Pointer auf die Wurzel des Baums
 */

/*!\var AVLTree::count
 * \brief Anzahl Knoten im Baum
 */

/*!\var AVLTree::current
 * \brief Aktueller Knoten beim Durchwandern des Baums
 */

/*!\var AVLTree::stack
 * \brief Alle Knoten oberhalb von current
 */

/*!\var AVLTree::stack_height
 * \brief Anzahl Knoten im Stack
 */

/*!\var AVLTree::controller
 * \brief Externes von CTreeController abgeleitetes Steuermodul
 */


__thread Heap *avlHeap=NULL;

/*!\brief Konstruktor
 *
 * \desc
 * Im Konstruktor wird der Baum mit NULL initialisiert und ein gemeinsamer
 * Heap-Speicher für die Verwaltung der Knoten-Elemente wird eingerichtet.
 */
AVLTree::AVLTree()
{
	MyHeap.init(sizeof(TREEITEM),0,100);
	controller=0;
	numElements=0;
	root=NULL;
	current=NULL;
	stack_height=0;
	dupes=false;
}

/*!\brief Destruktor
 *
 * \desc
 * Der Destruktor ruft die CTree::Clear Funktion auf.
 * \attention
 * Bei einer abgeleiteten Klasse muss der Destruktor ebenfalls implementiert werden und die Funktion
 * AVLTree::Clear aufrufen, um sicherzustellen, dass alle Elemente gelöscht werden.
 */
AVLTree::~AVLTree()
{
	clear();
}



int	AVLTree::compare(const void *value1, const void *value2) const
{
	if (controller) return controller->compare(value1, value2);
	throw NoTreeControllerException();
}


int AVLTree::getValue(const void *item, String &buffer) const
{
	if (controller) return controller->getValue(item, buffer);
	throw NoTreeControllerException();
}

int AVLTree::destroyValue(void *item) const
{
	if (controller) return controller->destroyValue(item);
	throw NoTreeControllerException();
}

/*!\brief Duplikate erlauben
 *
 * \desc
 * Mit dieser Funktion kann festgelegt werden, ob Elemente mit gleichem Schlüssel im Baum
 * erlaubt sind. Normalerweise ist dies nicht der Fall. Dabei gilt zu beachten, dass einige
 * Funktionen möglicherweise unerwartet funktionieren. So wird AVLTree::Find immer nur das
 * erste Element finden können, ebenso AVLTree::Delete oder AVLTree::Remove.
 *
 * @param allow Mit \c true werden Duplikate erlaubt, mit \c false nicht. Werden bei einem
 * bereits gefüllten Baum nachträglich Duplikate verboten, hat dies keine Auswirkung auf
 * bereits vorhandene Duplikate.
 */
void AVLTree::allowDupes(bool allow)
{
	dupes=allow;
}

/*!\brief Kontrollklasse festlegen
 *
 * \desc
 * Bei Verwendung der AVLTree-Klasse wird üblicherweise eine eigene Klasse davon abgeleitet und
 * mindestens die virtuelle Funktion AVLTree::Compare, optional auch AVLTree::DestroyValue und
 * AVLTree::GetValue reimplementiert. Es gibt jedoch auch die Möglichkeit AVLTree ohne
 * Ableitung zu verwenden. Dazu muss die eigene Klasse von CTreeController abgeleitet werden
 * und dessen Pointer mit dieser Funktion an die AVLTree-Klasse übergeben werden.
 *
 * @param[in] c Pointer auf eine von CTreeController abgeleitete Klasse, die mindestens die
 * virtuelle Funktion CTreeController::Compare implementiert hat.
 */
void AVLTree::setTreeController(AVLTreeController *c)
{
	controller=c;
}


/*!\brief Anzahl Elemente im Baum
 *
 * \desc
 * Diese Funktion liefert die Anzahl Elemente im Baum zurück.
 *
 * \return Anzahl Elemente, bzw. 0, wenn der Baum leer ist.
 */
size_t AVLTree::num() const
{
	return numElements;
}

/*!\brief Anzahl Elemente im Baum
 *
 * \desc
 * Diese Funktion liefert die Anzahl Elemente im Baum zurück.
 *
 * \return Anzahl Elemente, bzw. 0, wenn der Baum leer ist.
 */
size_t AVLTree::count() const
{
	return numElements;
}

/*!\brief Baum leeren
 *
 * \desc
 * Mit dieser Funktion wird der Inhalt des Baums gelöscht und sämtlicher durch die Elemente
 * belegter Speicher wieder freigegeben. Dazu wird für jedes Element zunächst AVLTree::DeleteNode und
 * dann die virtuelle Funktion AVLTree::DestroyValue aufgerufen. Zuletzt wiird noch der duch den
 * Knoten selbst belegte Speicher wieder freigegeben.
 * \attention
 * Bei einer abgeleiteten Klasse muss diese Funktion durch den Destruktor aufgerufen werden,
 * um sicherzustellen, dass alle Elemente gelöscht werden.
 *
 */
void AVLTree::clear()
{
	//printf ("Clear wurde aufgerufen, root ist: %u\n",root);
	TREEITEM *node;
	while (root) {
		//printf ("Root ist: %u\n",root);
		node=root;
		deleteNode(node);		// Element aus dem Baum entfernen
		if (node->data)	destroyValue((void*)node->data);	// Vom Inhalt belegten Speicher freigeben
		MyHeap.free(node);			// Vom Knoten belegten Speicher freigeben
	}
	root=NULL;
	numElements=0;
	current=NULL;
	stack_height=0;
}

/*!\brief Elemente des Baumes ausgeben
 *
 * \desc
 * Die Elemente des AVL-Baums werden auf der Konsole ausgegeben. Die Funktion ist zu
 * Debug-Zwecken vorhanden.
 *
 * @param node NULL oder Pointer auf Startknoten
 */
void AVLTree::printNodes(const TREEITEM *node) const
{
	String Buffer;
	if (!node) {
		if (!root) {
			PrintDebug ("AVLTree is empty\n");
			return;
		}
		node=root;
	}
	if (node->left) printNodes(node->left);
	if (getValue(node->data,Buffer)) Buffer.printnl();
	if (node->right) printNodes(node->right);

}

/*!\brief Wert im Baum finden
 *
 * \desc
 * Mit dieser Funktion wird der Wert \p value innerhalb des Baums gesucht und dessen
 * Baumelement (TREEITEM) zurückgegeben.
 *
 * \param[in] value Der zu suchende Wert
 * \return Wird der Wert im Baum gefunden, gibt die Funktion einen Pointer auf den
 * Wert des Baum-Elements zurück. Falls nicht, wird NULL zurückgegeben.
 */
TREEITEM *AVLTree::findNode(const void *value) const
{
	TREEITEM *cur=root;
	int cmp;
	while (cur) {
		cmp=compare(cur->data,value);
		if (cmp==-1) cur=cur->left;
		else if (cmp==1) cur=cur->right;
		else return cur;
	}
	return NULL;
}

/*!\brief Element im Baum finden
 *
 * \desc
 * Mit dieser Funktion wird ein Element innerhalb des Baums gesucht. Dazu wird
 * die Funktion CTree::CompareItems verwendet.
 *
 * \param[in] value Der zu suchende Wert
 * \return Wird das Element im Baum gefunden, gibt die Funktion einen Pointer auf den
 * Wert des Baum-Elements zurück. Falls nicht, wird NULL zurückgegeben.
 */
void *AVLTree::find(const void *value) const
{
	TREEITEM *cur=root;
	int cmp;
	while (cur) {
		cmp=compare(cur->data,value);
		if (cmp==-1) cur=cur->left;
		else if (cmp==1) cur=cur->right;
		else return (void*)cur->data;
	}
	return NULL;
}


/*!\brief Element hinzufügen
 *
 * \desc
 * Diese Funktion fügt ein neues Element in den Baum ein. Dabei ist sichergestellt,
 * dass der Baum stets sortiert und ausgewogen ist. Im Gegensatz zu CTree erlaub AVLTree
 * auch mehrere Elemente mit dem gleichen Schlüssel. Dieses Feature muss jedoch explizit
 * durch Aufruf der Funktion AVLTree::AllowDupes aktiviert werden.
 *
 * \param[in] value Pointer auf den hinzuzufügenden Wert
 * \return Wurde das Element erfolgreich hinzugefügt, gibt die Funktion true (1) zurück,
 * sonst false (0) und ein entsprechender Fehlercode wird gesetzt,
 * \exception EmptyDataException: Wird geworfen, wenn der Parameter \p value auf NULL zeigt
 * \exception OutOfMemoryException: Keine Speicher mehr verfügbar
 */
void AVLTree::add(const void *value)
{
	TREEITEM *cur=root;
	TREEITEM *node=NULL;
	int cmp=0;
	if (!value) throw EmptyDataException();
	TREEITEM *item=(TREEITEM *)MyHeap.malloc();
	if (!item) throw OutOfMemoryException();
	item->data=value;
	item->right=item->left=NULL;
	item->balance=0;
	if (!root) {
		root=item;
		item->parent=NULL;
		numElements++;
		return;
	}
	while (cur) {
		node=cur;
		cmp=compare(cur->data,value);
		if (cmp==0 && dupes==false) {
			MyHeap.free(item);
			throw DuplicateItemException();
		} else if (cmp<0) cur=cur->left;
		else cur=cur->right;
	}
	item->parent=node;
	if (cmp>0) {
		node->right=item;
		node->balance++;
	} else {
		node->left=item;
		node->balance--;
	}
	numElements++;
	if (node->balance!=0) upInsert(node);
}

/*!\brief Element anhand eines Wertes löschen
 *
 * \desc
 * Diese Funktion sucht zunächst ob der angegebene Wert im Baum enthalten ist und
 * löscht anschließend das gefundene Element. Dabei ist sichergestellt,
 * dass der Baum stets sortiert und ausgewogen ist.
 * \par
 * Beim Löschen des Elements wird auch die virtuelle Funktion AVLTree::DestroyValue
 * aufgerufen. Diese muss sicherstellen, dass der durch das Element belegte Speicher
 * ebenfalls freigegeben wird.
 * \par Falls das Element nur aus dem Baum entfernt werden, aber der durch das Element
 * belegte Speicher erhalten bleiben soll, kann stattdessen die Funktion AVLTree::Remove
 * aufgerufen werden.
 *
 * \param[in] value Pointer auf den zu löschenden Wert
 * \returns Wurde das Element erfolgreich gelöscht, gibt die Funktion true (1) zurück,
 * sonst false (0) und ein entsprechender Fehlercode wird gesetzt.
 */
void AVLTree::erase(const void *value)
{
	TREEITEM *item=findNode(value);
	if (!item) throw ItemNotFoundException();
	if (deleteNode(item)) {
		if (item->data)	destroyValue((void*)item->data);
		MyHeap.free(item);
		return;
	}
	throw DeleteFailedException();
}

/*!\brief Element aus dem AVL-Baum entfernen
 *
 * \desc
 * Diese Funktion sucht zunächst ob der angegebene Wert im Baum enthalten ist und
 * löscht anschließend das gefundene Element aus dem Baum. Dabei ist sichergestellt,
 * dass der Baum stets sortiert und ausgewogen ist.
 * \par
 * Mit dieser Funktion wird das Element nur aus dem AVL-Baum entfernt, der durch das Element
 * belegte Speicher wird jedoch nicht freigegeben. Falls dies gewünscht ist, kann stattdessen
 * die Funktion AVLTree::Delete aufgerufen werden.
 *
 * \param[in] value Pointer auf den zu entfernenden Wert
 * \returns Wurde das Element erfolgreich entfernt, gibt die Funktion true (1) zurück,
 * sonst false (0) und ein entsprechender Fehlercode wird gesetzt.
 */
void AVLTree::remove(const void *value)
{
	TREEITEM *item=findNode(value);
	if (!item) throw ItemNotFoundException();
	if (deleteNode(item)) {
		MyHeap.free(item);
		return;
	}
	throw DeleteFailedException();
}

/*!\brief Interne Funktion, die das tatsächliche Löschen vornimmt
 *
 * Dies ist eine interne Funktion, die den tatsächlichen Löschvorgang aus dem AVL-Baum vornimmt.
 * Der Inhalt des Knotens \p item und auch der Knoten selbst werden dabei nicht
 * gelöscht, dies muss die Aufrufende Funktion vornehmen. Sie wird von AVLTree::Delete aufgerufen.
 *
 * \param[in] item Pointer auf die Verwaltungsstruktur des Baumelements
 * \return Wurde das Element erfolgreich gelöscht, gibt die Funktion true (1) zurück,
 * sonst false (0) und ein entsprechender Fehlercode wird gesetzt,
 *
 */
int AVLTree::deleteNode(TREEITEM *item)
{
	TREEITEM *father;
	if((item->left)&&(item->right)) {
		TREEITEM *sys;
		if (item->balance<0) {
			// Wenn die Balance -1 ist, tauschen wir den Knoten mit dem am weitesten
			// rechts befindlichen Knoten des linken Teilbaums
			sys=item->left;	// Blatt finden
			while (sys->right) sys=sys->right;
		} else {
			// Wenn die Balance >=0 ist, tauschen wir den Knoten mit dem am weitesten
			// links befindlichen Knoten des rechten Teilbaums
			sys=item->right;	// Blatt finden
			while (sys->left) sys=sys->left;
		}
		swapNodes(item,sys);
		if (item==root) root=sys;
	}
	father=item->parent;
	if((!item->left)&&(!item->right)) {
		if(item==root) {
			//DestroyValue((void*)item->data);
			//Heap->Free(item);
			root=NULL;
			numElements--;
			return(1);
		} else {
			if(father->left==item) {
				father->left=NULL;
				father->balance++;
			} else {
				father->right=NULL;
				father->balance--;
			}
			upDelete(father);
			//DestroyValue((void*)item->data);
			//Heap->Free(item);
			numElements--;
			return 1;
		}
	}
	TREEITEM *sohn;
	if(item->left)
		sohn=item->left;
	else
		sohn=item->right;
	if(item!=root) {
		sohn->parent=father;
		if(father->left==item) {
			father->left=sohn;
			father->balance++;
		} else {
			father->right=sohn;
			father->balance--;
		}
		upDelete(father);
	} else {
		sohn->parent=NULL;
		root=sohn;
	}
	//DestroyValue((void*)item->data);
	//Heap->Free(item);
	numElements--;
	return 1;
}


void AVLTree::upInsert(TREEITEM *node)
{
	if (((node->balance==-1) || (node->balance==1)) && (node!=root)) {
		if (node->parent->left==node)
			node->parent->balance--;
		else
			node->parent->balance++;
		upInsert(node->parent);
		return;
	}
	if (node->balance==-2) {
		if (node->left->balance==-1) {
			rotate(node);
			return;
		} else {
			rotate(node->left);
			rotate(node);
			return;
		}
	}
	if (node->balance==2) {
		if (node->right->balance==1) {
			rotate(node);
			return;
		} else {
			rotate(node->right);
			rotate(node);
			return;
		}
	}
}

void AVLTree::upDelete(TREEITEM *node)
{
	TREEITEM *parent=node->parent;
	if ((node->balance==-1) || (node->balance==1)) return;
	if ((node==root) && (node->balance==0)) return;
	if (node==root) {
		if(node->balance==-2) {
			if (node->left->balance<=0) rotate(node);
			else {
				node=rotate(node->left);
				rotate(node);
			}
		} else {
			if (node->right->balance>=0) rotate(node);
			else {
				node=rotate(node->right);
				rotate(node);
			}
		}
		return;
	}
	if (node->balance==2) {
		switch(node->right->balance) {
			case 0:
				rotate(node);
				return;
			case 1:
				upDelete(rotate(node));
				return;
			case -1:
			rotate(node->right);
			upDelete(rotate(node));
			return;
		}
	}
	if (node->balance==-2) {
		switch(node->left->balance) {
			case 0:
				rotate(node);
				return;
			case -1:
			upDelete(rotate(node));
			return;
			case 1:
				rotate(node->left);
				upDelete(rotate(node));
				return;
		}
	}
	if (parent->left==node) {
		parent->balance++;
		if (parent->balance<2) {
			upDelete(parent);
			return;
		}
		switch(parent->right->balance) {
			case 0:
				rotate(parent);
				return;
			case 1:
				upDelete(rotate(parent));
				return;
			case -1:
				rotate(parent->right);
				upDelete(rotate(parent));
				return;
		}
	}
	if (parent->right==node) {
		parent->balance--;
		if (parent->balance>-2) {
			upDelete(parent);
			return;
		}
		switch (parent->left->balance) {
			case 0:
				rotate(parent);
				return;
			case -1:
				upDelete(rotate(parent));
				return;
			case 1:
				rotate(parent->left);
				upDelete(rotate(parent));
				return;
		}
	}
}

TREEITEM *AVLTree::rotate(TREEITEM *kn)
{
	TREEITEM *child;
	if (kn->balance<0) {
		child=kn->left;
		kn->left=child->right;
		if (child->right) child->right->parent=kn;
		child->right=kn;
		child->parent=kn->parent;
		kn->parent=child;
		if (child->parent) {
			if (child->parent->left==kn)
				child->parent->left=child;
			else
				child->parent->right=child;
		} else
			root=child;
		if (kn->balance==-1) {
			if(child->balance==1) {
				child->balance=2;
				kn->balance=0;
				return child;
			}
			if (child->balance==-1)
				kn->balance=1;
			else
				kn->balance=0;
			child->balance=1;
			return child;
		}
		if (kn->balance==-2) {
			if (child->balance==-1) {
				kn->balance=child->balance=0;
				return child;
			}
			if(child->balance==0) {
				kn->balance=-1;
				child->balance=1;
				return child;
			}
			if (child->balance==-2) {
				kn->balance=1;
				child->balance=0;
				return child;
			}
		}
	} else {
		child=kn->right;
		kn->right=child->left;
		if(child->left) child->left->parent=kn;
		child->left=kn;								// Step 3
		child->parent=kn->parent;					// Step 4
		kn->parent=child;							// Step 5
		if(child->parent) {
			if (child->parent->left==kn)
				child->parent->left=child;
			else
				child->parent->right=child;
		} else
			root=child;
		if (kn->balance==1) {
			if (child->balance==-1) {
				child->balance=-2;
				kn->balance=0;
				return child;
			}
			if (child->balance==1)
				kn->balance=-1;
			else
				kn->balance=0;
			child->balance=-1;
			return child;
		}
		if (kn->balance==2) {
			if (child->balance==1) {
				kn->balance=child->balance=0;
				return child;
			}
			if (child->balance==0) {
				kn->balance=1;
				child->balance=-1;
				return child;
			}
			if (child->balance==2) {
				kn->balance=-1;
				child->balance=0;
				return child;
			}
		}
	}
	return child;
}

void AVLTree::swapNodes(TREEITEM *item1, TREEITEM *item2)
{
	signed char	balance;
	TREEITEM *left, *right, *parent;
	// Kopieren der Parameter von item1 nach tmp
	left=item1->left;
	right=item1->right;
	balance=item1->balance;
	parent=item1->parent;
	// Kopieren der Parameter von item2 nach item1
	item1->left=item2->left;
	item1->right=item2->right;
	item1->balance=item2->balance;
	item1->parent=item2->parent;
	// Kopieren der Parameter von tmp nach item2
	item2->left=left;
	item2->right=right;
	item2->balance=balance;
	item2->parent=parent;
	// Falls die Nodes direkt nebeneinander liegen, könnten die Child-Pointer auf sich selbst zeigen.
	// Das müssen wir korrigieren
	if (item1->left==item1) item1->left=item2;
	if (item1->right==item1) item1->right=item2;
	if (item2->left==item2) item2->left=item1;
	if (item2->right==item2) item2->right=item1;
	// Pointer der Kinder korrigieren
	if (item1->left) item1->left->parent=item1;
	if (item1->right) item1->right->parent=item1;
	if (item2->left) item2->left->parent=item2;
	if (item2->right) item2->right->parent=item2;
	// Pointer der Parents korrigieren
	if (item1->parent) {
		if (item1->parent->left==item2) item1->parent->left=item1;
		else if (item1->parent->right==item2) item1->parent->right=item1;
	}
	if (item2->parent) {
		if (item2->parent->left==item1) item2->parent->left=item2;
		else if (item2->parent->right==item1) item2->parent->right=item2;
	}
}



AVLTree::Iterator::Iterator()
{
	current=NULL;
	stack_height=0;
}

/*!\brief Iterator auf erstes Element zurücksetzen
 *
 * \desc
 *
 * @param it Iterator
 */
void AVLTree::reset(Iterator &it) const
{
	it.current=NULL;
	it.stack_height=NULL;
}

/*!\brief Erstes Element aus dem Baum
 *
 * \desc
 * Mit dieser Funktion wird ein Pointer auf das erste Element des Baums zurückgeliefert.
 *
 * @param it Iterator
 *
 * \return Pointer auf das erste Element des Baums oder NULL, wenn der Baum leer ist
 */
void *AVLTree::getFirst(Iterator &it) const
{
	it.current=NULL;
	it.stack_height=0;
	TREEITEM *node;
	node = root;
	if (node != NULL)
		while (node->left != NULL) {
    		it.stack[it.stack_height++] = node;
			node = node->left;
		}
	it.current = node;
	if (node) return (void*)node->data;
	return NULL;
}

/*!\brief Nächstes Element aus dem Baum
 *
 * \desc
 * Mit dieser Funktion wird ein Pointer auf das nächste Element des Baums zurückgeliefert.
 * Somit kann der Baum sortiert vorwärts durchwandert werden.
 *
 * @param it Iterator
 *
 * \return Pointer auf das nächste Element des Baums oder NULL, wenn keine weiteren
 * Elemente vorhanden sind.
 */
void *AVLTree::getNext(Iterator &it) const
{
	TREEITEM *node=it.current, *tmp;
	if (node == NULL) {
		return getFirst(it);
	} else if (node->right != NULL) {
		it.stack[it.stack_height++] = node;
		node = node->right;
		while (node->left != NULL) {
			it.stack[it.stack_height++] = node;
			node = node->left;
		}
	} else {
		do {
			if (it.stack_height == 0) {
				it.current = NULL;
				return NULL;
			}
			tmp=node;
			node = it.stack[--it.stack_height];
		} while (tmp == node->right);
	}
	it.current=node;
	if (node) return (void*)node->data;
	return NULL;
}

/*!\brief Letztes Element aus dem Baum
 *
 * \desc
 * Mit dieser Funktion wird ein Pointer auf das letzte Element des Baums zurückgeliefert.
 *
 * @param it Iterator
 *
 * \return Pointer auf das letzte Element des Baums oder NULL, wenn der Baum leer ist
 */
void *AVLTree::getLast(Iterator &it) const
{
	TREEITEM *node;
	it.stack_height = 0;
	node = root;
	if (node != NULL)
		while (node->right != NULL) {
			it.stack[it.stack_height++] = node;
			node = node->right;
		}
	it.current = node;
	if (node) return (void*)node->data;
	return NULL;
}

/*!\brief Vorheriges Element aus dem Baum
 *
 * \desc
 * Mit dieser Funktion wird ein Pointer auf das vorherige Element des Baums zurückgeliefert.
 * Somit kann der Baum sortiert rückwärts durchwandert werden.
 *
 * @param it Iterator
 *
 * \return Pointer auf das vorherige Element des Baums oder NULL, wenn keine weiteren
 * Elemente vorhanden sind.
 */
void *AVLTree::getPrevious(Iterator &it) const
{
	TREEITEM *node, *y;
	node = it.current;
	if (node == NULL) {
		return getLast(it);
	} else if (node->left != NULL) {
		it.stack[it.stack_height++] = node;
		node = node->left;
		while (node->right != NULL) {
			it.stack[it.stack_height++] = node;
			node=node->right;
		}
	} else {
		do {
			if (it.stack_height == 0) {
				it.current = NULL;
				return NULL;
			}
			y = node;
			node = it.stack[--it.stack_height];
		} while (y == node->left);
	}
	it.current = node;
	if (node) return (void*)node->data;
	return NULL;
}

/*!\brief Aktuelles Element des Baums
 *
 * \desc
 * Mit dieser Funktion wird ein Pointer auf das aktuelle Element des Baums zurückgeliefert.
 * Dabei wird der Pointer nicht verändert.
 *
 * @param it Iterator
 *
 * \return Pointer auf das aktuelle Element des Baums oder NULL, wenn kein
 * Element mehr vorhanden ist.
 */
void *AVLTree::getCurrent(Iterator &it) const
{
	if (it.current) return (void*)it.current->data;
	return NULL;
}


}	// EOF namespace ppl7
