#ifndef __LIST_H__
#define __LIST_H__

#include <stdlib.h>
#include <sys/types.h>
#include "status.h"

/** Struktura węzła listy dwukierunkowej.
  */
typedef struct Node {
    struct Node *prev;
    struct Node *next;
    int value;
} Node;

/** Struktura listy dwukierunkowej z dowiązaniami.
  */
typedef struct List {
    Node *begin;
    Node *end;
} List;

/** @brief Tworzy strukturę nowej listy.
 * @return Wskaźnik na nową listę, lub NULL gdy nie powiodła się alokacja.
 */
List* newList(void);

/** @brief Wstawia element @p elem do listy po węźle @p node.
 * Wstawia element @p elem do listy po węźle @p node. Jeśli @p node jest równe list->end, takie wywołanie jest
 * równoważne z listInsertAfter(list, list->end->prev, elem).
 * @param[in,out] list         - lista, do której wstawiamy
 * @param[in] node             - węzeł, za którym wstawiamy element
 * @param[in] elem             - element do wstawienia
 * @return Status powodzenia wstawienia. Operacja może się nie powieść, jeśli nie uda się zaalokować pamięci.
 */
Status listInsertAfter(List* list, Node* node, int elem);

/** @brief Wstawia element @p elem do listy przed węzłem @p node.
 * Wstawia element @p elem do listy przed węzłem @p node. Jeśli @p node jest równe list->begin, takie wywołanie jest
 * równoważne z listInsertBefore(list, list->begin->next, elem).
 * @param[in,out] list         - lista, do której wstawiamy
 * @param[in] node             - węzeł, przed którym wstawiamy element
 * @param[in] elem             - element do wstawienia
 * @return Status powodzenia wstawienia. Operacja może się nie powieść,
 * jeśli nie uda się zaalokować pamięci.
 */
Status listInsertBefore(List* list, Node* node, int elem);

/** @brief Usuwa węzeł z listy.
 * Jeśli węzeł nie znajduje się na liście, operacja jest nieokreślona.
 * @param list[in,out]         - lista, z której usuwamy element
 * @param node[in]             - wskaźnik na węzeł, który usuwamy z listy
 */
void deleteListNode(List* list, Node* node);

/** @brief Usuwa listę.
 * Usuwa listę, dealokując całą używaną pamięć.
 * @param list[in,out]         - lista do usunięcia.
 */
void deleteList(List* list);

/** @brief Tworzy kopię listy.
 * @param list[in]             - lista, której kopię tworzymy
 * @return wskaźnik na nową listę, lub NULL w przypadku błędu alokacji pamięci
 */
List* copyList(List* list);

/** @brief Wstaw element opisany przez wskaźnik @p val po węźle @p after.
 * Operacja nie dokkonuje alokacji pamięci. Wynik nie jest określony, jeśli @p after
 * nie znajduje się na liście.
 * @param list[in,out]         - lista, na której znajduje się @p after
 * @param after[in,out]        - element, po krórym wstawiamy @p val
 * @param val[in,out]          - element wstawiany
 */
void listEmplaceNode(List* list, Node* after, Node* val);

/** @brief Wstawia listę @p new do listy @l po elemencie o wartości @p el. Jeśli
 * @p el znajduje się na liście @p new, inwaliduje kolejne operacje na liście
 * @p new (i przejmuje obowiązek zajmowania się zwalnianiem pamięci).
 * @param l[in,out]            - lista, którą będziemy poszerzać
 * @param new[in,out]          - lista, którą będziemy wstawiać
 * @param el[in]               - element, po którym chcemy wstawić listę @p new
 */
void insertListAfterElement(List* l, List* new, int el);

/** @brief Znajduje pozycję pierwszego elementu @p el w liście @p l.
 * Jeśli nie znaleziono elementu, zwraca -1.
 * @param l[in]                - lista, którą przeszukujemy
 * @param el[in]               - szukany element
 * @return
 */
ssize_t listPos(List* l, int el);

/** @brief Odwraca listę w miejscu.
 * @param l[in,out]            - lista do odwrócenia
 */
void listReverse(List* l);

/** @brief Znajduje pierwszy węzeł o wartości @p el w liście @p l.
 * @param l[in]                - lista, którą przeszukujemy
 * @param el[in]               - szukany element
 * @return NULL jeśli l == NULL lub element o wartości @p el nie istnieje na liście.
 */
Node* listFind(List* l, int el);

#endif /* __LIST_H__ */
