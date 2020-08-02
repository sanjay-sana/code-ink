#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include "my402list.h"

typedef struct tagMy402ListElement
{
	char *type;
	long int timestamp;
	float amount;
	char *description;
}My402ListElement;

void printStatement(My402List* linkedList)
{
	fprintf(stdout, "+-----------------+--------------------------+----------------+----------------+\n");
	fprintf(stdout, "|       Date      | Description              |         Amount |        Balance |\n");
	fprintf(stdout, "+-----------------+--------------------------+----------------+----------------+\n");
	My402ListElem *tempElement = My402ListFirst(linkedList);
	My402ListElement *tempObject; 
	for (int i = 0; i < linkedList->num_members; ++i)
	{
		tempObject = (My402ListElement*)tempElement->obj;
		fprintf(stdout, "%s %ld %.2f %s\n", tempObject->type, tempObject->timestamp, tempObject->amount, tempObject->description);
		tempElement = My402ListNext(linkedList, tempElement);
	}
	fprintf(stdout, "+-----------------+--------------------------+----------------+----------------+\n");
	// fprintf(stdout, "%s %ld %.2f %s\n", tempObject->type, tempObject->timestamp, tempObject->amount, tempObject->description);
	// My402ListElem *nextElement1 = My402ListNext(linkedList, tempElement);
	// My402ListElement *tempObject1 = (My402ListElement*)nextElement1->obj;
	// fprintf(stdout, "%s %ld %.2f %s\n", tempObject1->type, tempObject1->timestamp, tempObject1->amount, tempObject1->description);
}

void readInput(FILE *fileInput, My402List *linkedList)
{
	char buf[1026];
	// char tempbuf[30];
	while (fgets(buf, sizeof(buf), fileInput) != NULL)
	{
		fprintf(stdout, "%s\n", buf);
		My402ListElement *newline;
		newline = (My402ListElement*)malloc(sizeof(My402ListElement));
		char *start_ptr = buf;
		char *tab_ptr = strchr(start_ptr, '\t');
		if (tab_ptr != NULL)
		{
			*tab_ptr++ = '\0';
			// fprintf(stdout, "hell1:%s", tab_ptr);
			// fprintf(stdout, "hell1:%s\n", start_ptr);
			newline->type = start_ptr;
			if (*(newline->type) != '+' && *(newline->type) != '-')
			{
				fprintf(stderr, "Invalid transaction type in the input file\n");
			}
			start_ptr = tab_ptr;
			tab_ptr = strchr(start_ptr, '\t');
			if (tab_ptr != NULL)
			{
				*tab_ptr++ = '\0';
				// fprintf(stdout, "hell2:%s", tab_ptr);
				// fprintf(stdout, "hell2:%s\n", start_ptr);
				newline->timestamp = atol(start_ptr);
				// fprintf(stdout, "yess\n");
				start_ptr = tab_ptr;
				tab_ptr = strchr(start_ptr, '\t');
				if (tab_ptr != NULL)
				{
					*tab_ptr++ = '\0';
					// fprintf(stdout, "hell3:%s", tab_ptr);
					// fprintf(stdout, "hell3:%s\n", start_ptr);
					// strcpy(newline->amount, start_ptr);
					newline->amount = atof(start_ptr);
					newline->description = tab_ptr;
					fprintf(stdout, "Count before: %d\n", linkedList->num_members);
					// fprintf(stdout, "%s\n", newline->type);
					// fprintf(stdout, "%ld\n",newline->timestamp);
					// fprintf(stdout, "%f\n", newline->amount);
					// fprintf(stdout, "%s\n", newline->description);

					// void *object1 = (void*)newline;
					// My402ListElement *checkobject = (My402ListElement*)object1;
					// fprintf(stdout, "object1:%p\n", newline);
					// fprintf(stdout, "object2:%p\n", object1);
					// fprintf(stdout, "obj:%s\n", checkobject->type);
					// fprintf(stdout, "obj:%ld\n", checkobject->timestamp);
					// fprintf(stdout, "obj:%f\n", checkobject->amount);
					// fprintf(stdout, "obj:%s\n", checkobject->description);
					int succ = 0;
					if (My402ListEmpty(linkedList))
					{
						succ = My402ListAppend(linkedList, (void*)newline);
						fprintf(stdout, "Zero if\n");
					}
					else
					{
						My402ListElem *firstElement, *lastElement, *nextElement;
						firstElement = My402ListFirst(linkedList);
						lastElement = My402ListLast(linkedList);
						int insertFlag = 0;
						My402ListElement *tempObjectFirst = (My402ListElement*)firstElement->obj;
						// My402ListElement *tempObjectLast = (My402ListElement*)lastElement->obj;
						if (newline->timestamp < tempObjectFirst->timestamp)
						{
							fprintf(stdout, "First if\n");
							My402ListInsertBefore(linkedList, (void*)newline, firstElement);
							succ = 1;
						}
						else
						{
							// fprintf(stdout, "Yee\n");
							nextElement = My402ListNext(linkedList, firstElement);
							// fprintf(stdout, "Yeeee\n");
							if (nextElement == NULL)
							{
								My402ListAppend(linkedList, (void*)newline);
								succ = 1;
								fprintf(stdout, "Second if\n");
							}
							else
							{
								My402ListElement *tempObjectNext = (My402ListElement*)nextElement->obj;
								for (int i = 1; i < linkedList->num_members; i++)
								{
									if ((newline->timestamp < tempObjectNext->timestamp) && insertFlag == 0)
									{
										fprintf(stdout, "Third if\n");
										My402ListInsertBefore(linkedList, (void*)newline, nextElement);
										insertFlag = 1;
										succ = 1;
									}
								}
							}
							// if ((newline->timestamp > tempObjectLast->timestamp) && insertFlag == 0)
							// {
							// 	fprintf(stdout, "Third if\n");
							// 	My402ListAppend(linkedList, newline);
							// 	succ = 1;
							// }
						}
					}
					fprintf(stdout, "Count after: %d\n", linkedList->num_members);
					if (succ == 0)
					{
						fprintf(stderr, "Unable to add data from input file\n");
					}
					fprintf(stdout, "Final List\n");
					printStatement(linkedList);
				}
			}
		}
	}
}

int main(int argc, char *argv[])
{
	FILE *fileInput = NULL;
	if (argc == 3)
	{
		if (strcmp(argv[1],"sort") != 0)
		{
			fprintf(stderr, "Please enter sort after the program name.\n");
		}
		fileInput = fopen(argv[2], "r");
		if (fileInput == NULL)
		{
			fprintf(stderr, "Cannot open the file %s\n", argv[2]);
		}
	}
	else if (argc == 2)
		{
			if (strcmp(argv[1],"sort") != 0)
			{
				fprintf(stderr, "Please enter sort after the program name.\n");
			}
			fileInput = stdin;
		}
		else
		{
			fprintf(stderr, "Incorrect command line arguments\n");
		}
	My402List *linkedList;
	linkedList = (My402List*)malloc(sizeof(My402List));
	(void)My402ListInit(linkedList);
	readInput(fileInput, linkedList);
	// if (!readInput(fileInput, &linkedList))
	// {
	// 	fprintf(stderr, "Error in read input\n");
	// }
	// printStatement(linkedList);
	if (fileInput != NULL)
	{
		fclose(fileInput);
	}	
	return 0;
}
