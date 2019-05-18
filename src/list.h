/** @file
 * Interfejs dostarczajacy strukturę listy dwukierunkowej z dowiązaniami.
 */
#ifndef __LIST_H__
#define __LIST_H__

#include "status.h"
#include <stdlib.h>
#include <sys/types.h>

/** Struktura węzła listy dwukierunkowej.
 */
typedef struct Node {
    /// Wskaźnik na poprzedni element w liście.
    struct Node *prev;
    /// Wskaźnik na kolejny element w liście.
    struct Node *next;
    /// Wartość przechowywana w elemencie.
    int value;
} Node;

/** Struktura listy dwukierunkowej z dowiązaniami.
 * Lista przechowuje specjalne elementy, początek i koniec, w których nie przechowuje się żadnych wartości.
 */
typedef struct List {
    /// Wskaźnik na początek listy.
    Node *begin;
    /// Wskaźnik na koniec listy.
    Node *end;
} List;

/** @brief Tworzy strukturę nowej listy.
 * @return Wskaźnik na nową listę, lub NULL gdy nie powiodła się alokacja.
 */
List *newList(void);

/** @brief Wstawia element @p elem do listy po węźle @p node.
 * Wstawia element @p elem do listy po węźle @p node. Jeśli @p node jest równe
 * list->end, takie wywołanie jest równoważne z listInsertAfter(list,
 * list->end->prev, elem).
 * @param[in,out] list         - lista, do której wstawiamy
 * @param[in] node             - węzeł, za którym wstawiamy element
 * @param[in] elem             - element do wstawienia
 * @return Status powodzenia wstawienia. Operacja może się nie powieść, jeśli
 * nie uda się zaalokować pamięci.
 */
Status listInsertAfter(List *list, Node *node, int elem);

/** @brief Wstawia element @p elem do listy przed węzłem @p node.
 * Wstawia element @p elem do listy przed węzłem @p node. Jeśli @p node jest
 * równe list->begin, takie wywołanie jest równoważne z listInsertBefore(list,
 * list->begin->next, elem).
 * @param[in,out] list         - lista, do której wstawiamy
 * @param[in] node             - węzeł, przed którym wstawiamy element
 * @param[in] elem             - element do wstawienia
 * @return Status powodzenia wstawienia. Operacja może się nie powieść,
 * jeśli nie uda się zaalokować pamięci.
 */
Status listInsertBefore(List *list, Node *node, int elem);

/** @brief Usuwa węzeł z listy.
 * Jeśli węzeł nie znajduje się na liście, operacja jest nieokreślona.
 * @param[in,out] list         - lista, z której usuwamy element
 * @param[in] node             - wskaźnik na węzeł, który usuwamy z listy
 */
void deleteListNode(List *list, Node *node);

/** @brief Usuwa listę.
 * Usuwa listę, dealokując całą używaną pamięć.
 * @param[in,out] list         - lista do usunięcia.
 */
void deleteList(List *list);

/** @brief Tworzy kopię listy.
 * @param[in] list             - lista, której kopię tworzymy
 * @return wskaźnik na nową listę, lub NULL w przypadku błędu alokacji pamięci
 */
List *copyList(List *list);

/** @brief Wstaw element opisany przez wskaźnik @p val po węźle @p after.
 * Operacja nie dokkonuje alokacji pamięci. Wynik nie jest określony, jeśli @p
 * after nie znajduje się na liście.
 * @param[in,out] list         - lista, na której znajduje się @p after
 * @param[in,out] after        - element, po krórym wstawiamy @p val
 * @param[in,out] val          - element wstawiany
 */
void listEmplaceNode(List *list, Node *after, Node *val);

/** @brief Wstawia listę @p new do listy @p l po elemencie o wartości @p el.
 * Jeśli @p el znajduje się na liście @p new, inwaliduje kolejne operacje na liście
 * @p new (i przejmuje obowiązek zajmowania się zwalnianiem pamięci).
 * @param[in,out] l            - lista, którą będziemy poszerzać
 * @param[in,out] new          - lista, którą będziemy wstawiać
 * @param[in] el               - element, po którym chcemy wstawić listę @p new
 */
void insertListAfterElement(List *l, List *new, int el);

/** @brief Znajduje pozycję pierwszego elementu @p el w liście @p l.
 * Jeśli nie znaleziono elementu, zwraca -1.
 * @param[in] l                - lista, którą przeszukujemy
 * @param[in] el               - szukany element
 * @return
 */
ssize_t listPos(List *l, int el);

/** @brief Odwraca listę w miejscu.
 * @param[in,out] l            - lista do odwrócenia
 */
void listReverse(List *l);

/** @brief Znajduje pierwszy węzeł o wartości @p el w liście @p l.
 * @param[in] l                - lista, którą przeszukujemy
 * @param[in] el               - szukany element
 * @return NULL jeśli l == NULL lub element o wartości @p el nie istnieje na
 * liście.
 */
Node *listFind(List *l, int el);

/** @brief Sprawdza, czy lista jest pusta.
 * @param[in] l                - lista do sprawdzenia
 * @return @p true jeśli lista jest pusta, @p false gdy nie jest.
 */
bool isEmptyList(List* l);

#endif /* __LIST_H__ */
