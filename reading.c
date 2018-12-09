#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <dirent.h>
#include <sys/types.h>
#include <errno.h>

typedef struct str_node *str_list;
struct str_node
{
	char* str;
	str_list next;
};

str_list str_add(str_list list, const char* str)
{
	str_list iter = list;
	int str_size = strlen(str) + 1;
	str_list new_node = malloc(sizeof(struct str_node));

	new_node->next = NULL;
	new_node->str = malloc(str_size);
	memcpy(new_node->str, str, str_size);

	if (list == NULL)
		return new_node;

	while (iter->next != NULL)
		iter = iter->next;

	iter->next = new_node;

	return list;
}

int make_arguments(str_list* list, char* str)
{
	char* special_symbols[] =
	{ ">>", ">", "<", "&", "|", "`" };
	int position = 0;
	int n = 0;
	int ret = 0;
	char* str1 = (char*) calloc(1, strlen(str) + 1);
	str_list L = *list;

	while (str[position] != 0)
	{
		int is_special = 0;

		for (int i = 0; i < sizeof(special_symbols) / sizeof(special_symbols[0]); i++)
		{
			if (strncmp(str + position, special_symbols[i], strlen(special_symbols[i])) == 0)
			{
				if (n > 0)
					L = str_add(L, str1);
				n = 0;
				memset(str1, 0, strlen(str) + 1 );
				position += strlen(special_symbols[i]);
				L = str_add(L, special_symbols[i]);
				is_special = 1;
				break;
			}
		}

		if( is_special )
			continue;

		if ((str[position] == ' ') || (str[position] == '\n'))
		{
			if (n > 0)
				L = str_add(L, str1);
			n = 0;
			memset(str1, 0, strlen(str) + 1 );
			position++;
		}
		else if (str[position] == '"')
		{
			int save_position = position;
			position++;
			while (str[position] != 0)
			{
				if (str[position] != '"')
				{
					str1[n++] = str[position++];
				}
				else
				{
					position++;
					break;
				}
			}

			if (str[position] == 0)
			{
				memmove(str, str + save_position, strlen(str + save_position) + 1);
				ret = 1;
				break;
			}
		}
		else
		{
			str1[n++] = str[position++];
		}
	}

	*list = L;
	free(str1);
	return ret;
}

int main()
{
	str_list arguments = NULL;
	char c;
	int n = 0;
	char* str = (char*) calloc(1, sizeof(char));
	char tmp_str[5];

	while (fgets(tmp_str, sizeof(tmp_str), stdin) != NULL)
	{
		int len = strlen(tmp_str);

		str = realloc(str, strlen(str) + len + 1);
		strcat(str, tmp_str);
		if (tmp_str[len - 1] == '\n')
		{
			if (make_arguments(&arguments, str) == 0)
			{
				free(str);
				str = (char*) calloc(1, sizeof(char));
			}
		}
	}
	while (arguments != NULL)
		{
			printf("%s\n", arguments->str);
			arguments = arguments->next;
		}
	return 0;
}

