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

typedef struct pid_node *pid_list;
struct pid_node
{
	pid_t pid;
	pid_list next;
};

typedef struct one_command *shell;
struct one_command {
	str_list comand;
	char* input;
	char* output;
	char* output_add;
	int fn;
	shell next_command;
};

/*Добавляет эелемент списка*/
pid_list pid_add(pid_list list, pid_t pid)
{
	pid_list iter = list;
	pid_list new_node = malloc(sizeof(struct pid_node));

	new_node->next = NULL;
	new_node->pid = pid;

	if (list == NULL)
		return new_node;

	while (iter->next != NULL)
		iter = iter->next;

	iter->next = new_node;

	return list;
}

/*Удаляет элемент списка*/
pid_list pid_del(pid_list list, pid_t pid)
{
	pid_list iter = list; /*Итератор*/
	pid_list prev = list; /*Предыдущий элемент*/

	while (iter != NULL)
	{
		if (iter->pid != pid)
		{
			prev = iter;
			iter = iter->next;
			continue;
		}

		if (iter == list) /*Удалить первый элемент списка*/
		{
			list = iter->next;
		}
		else
		{
			prev->next = iter->next;
		}
		free(iter);
		break;
	}

	return list;
}

/*Удаляет весь список, рекурсия*/
void pid_free(pid_list list)
{
	if (list == NULL)
		return;

	pid_free(list->next);
	free(list);
}

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

/*Удаляет весь список, рекурсия*/
void str_free(str_list list)
{
	if (list == NULL)
		return;

	str_free(list->next);
	free(list->str);
	free(list);
}

int str_size(str_list list)
{
	int size = 0;

	while (list != NULL)
	{
		size++;
		list = list->next;
	}

	return size;
}

shell make_shell(str_list L, shell Sh) {
	str_list Lst=L;
	while (L!=NULL) {
		if (L->str=='>'){
			shell->output=L->next->str;
			L=L->next->next;
		}
		else if (L->str=='<'){
			shell->input=L->next->str;
			L=L->next->next;
		}
		else if (L->str=='>>'){
			shell->output_add=L->str;
			L=L->next->next;
		}
		else if ()
	}
	return Sh;
}




int check_cd(str_list strings)
{
	char* dir; //Домашний каталог по умолчанию, если cd без аргументов
	if (strings == NULL)
		return 0;

	if (strcmp(strings->str, "cd") != 0)
		return 0;

	if (strings->next != NULL)
	{
		dir = strings->next->str;
	}
	else
		return 0;

	if (chdir(dir) != 0)
	{
		fprintf( stderr, "cd: \"%s\": %s\n", strings->next->str, strerror( errno));
	}

	return 1;
}

pid_list check_background_processes(pid_list list)
{
	pid_t pid;
	int status;

	while ((pid = waitpid(-1, &status, WNOHANG)) > 0)
	{
		if (WIFEXITED(status))
		{
			list = pid_del(list, pid);
			printf("process with PID=%d finished with status %d\n", pid, WEXITSTATUS(status));
		}
		else if (WIFSIGNALED(status))
		{
			list = pid_del(list, pid);
			printf("process with PID=%d terminated by signal %d\n", pid, WTERMSIG(status));
		}
	}

	return list;
}

pid_list run_process(str_list strings, pid_list pids)
{
	char** args;
	int nargs = str_size(strings);
	int background = 0;
	pid_t pid;

	if (check_cd(strings))
		return pids;

	args = malloc((nargs + 1) * sizeof(char*));
	nargs = 0;
	while (strings != NULL)
	{
		args[nargs] = strings->str;
		nargs++;
		strings = strings->next;
	}

	if (strcmp(args[nargs - 1], "&") == 0)
	{
		nargs--;
		background = 1;
	}

	args[nargs] = NULL;

	pid = fork();
	if (!pid)
	{

		execvp(args[0], args);
		fprintf( stderr, "Error executing %s: %s\n", args[0], strerror( errno));
		exit(1);
	}

	if (background)
	{
		pids = pid_add(pids, pid);
		printf("running the background process with PID=%d\n", pid);
	}
	else
	{
		int status;
		pid_t ret;

		while ((ret = waitpid(pid, &status, 0)) >= 0)
		{
			if (ret != pid)
				continue;

			if (WIFEXITED(status))
			{
				break;
			}
			else if (WIFSIGNALED(status))
			{
				printf("process with terminated by signal %d\n", WTERMSIG(status));
				break;
			}
		}
	}

	free(args);

	return pids;
}

void kill_background_processes(pid_list pids)
{
	pid_list iter = check_background_processes(pids); //проверим вдруг ктото уже завершился

	//остальных прибъем сигналом SIGKILL
	while (iter)
	{
		if (kill(iter->pid, SIGKILL) != 0)
			fprintf( stderr, "Can't kill process pid=%d: %s!\n", iter->pid, strerror( errno));

		iter = iter->next;
	}

	//проверим что все убитые завершились
	while ((pids = check_background_processes(pids)) != NULL)
		usleep(10000);
}

int main()
{
	shell Sh=NULL;
	pid_list background_pids = NULL;
	str_list arguments = NULL;
	char c;
	int n = 0;
	int fn_flag = 0;
	int arrow_flag = 0;
	char *s = (char*) calloc(1, sizeof(char));
	char arrow[2];
	arrow[0] = '>';
	arrow[1] = 0;

	while ((c = getchar()) != EOF)
	{                     //читаю слова
		if (c != '\n')
		{
			if (fn_flag)
			{
				fprintf( stderr, "\nWrong position of '&'!\n");
				str_free(arguments);
				arguments = NULL;
				n = 0;
				s = realloc(s, n + 1);
				s[n] = 0;
				fn_flag = 0;
				while (getchar() != '\n')
					;
				continue;
			}

			if (c == '"')
			{
				while ((c = getchar()) != '"')
				{
					if (c == EOF)
					{
						printf("\n");
						printf("problem\n");
						return 1;
					}
					s = realloc(s, n + 2);
					s[n++] = c;
					s[n] = 0;
				}
			}
			else if (c != ' ')
			{
				if (arrow_flag)
				{
					if (c == '>')
					{
						n = 2;
						s = realloc(s, n + 1);
						s[0] = '>';
						s[1] = '>';
						s[2] = 0;
						arguments = str_add(arguments, s);
						n = 0;
						s = realloc(s, n + 1);
						s[0] = 0;
						arrow_flag = 0;
					}
					else
					{
						arguments = str_add(arguments, arrow);
						n = 1;
						s = realloc(s, n + 1);
						s[0] = c;
						s[1] = 0;
						arrow_flag = 0;
					}
				}
				else
				{
					if ((c == '|') || (c == '<'))
					{
						if (n > 0)
							arguments = str_add(arguments, s);
						n = 1;
						s = realloc(s, n + 1);
						s[n] = 0;
						s[0] = c;
						arguments = str_add(arguments, s);
						n = 0;
						s = realloc(s, n + 1);
						s[0] = 0;

					}
					else if (c == '>')
					{
						if (n>0) arguments = str_add(arguments, s);
						n = 0;
						s = realloc(s, n + 1);
						s[0] = 0;
						arrow_flag = 1;
					}
					else
					{
						s = realloc(s, n + 2);
						s[n++] = c;
						s[n] = 0;
					}
					if (c == '&')
					{
						fn_flag = 1;
					}
				}
			}

			else
			{
				if ((!arrow_flag) && (n > 0))

				{
					arguments = str_add(arguments, s);
					n = 0;
					s = realloc(s, n + 1);
					s[0] = 0;
				}
				else if (strlen(s) > 0)
				{
					n = 1;
					s = realloc(s, n + 1);
					s[0] = '>';
					s[n] = 0;
					arguments = str_add(arguments, s);
					n = 0;
					s = realloc(s, n + 1);
					s[0] = 0;
					arrow_flag = 0;
				}

			}
		}
		else
		{
			if (arrow_flag)
			{
				arguments = str_add(arguments, arrow);
				arrow_flag = 0;
			}
			if (strlen(s) != 0)  // если не внесли в список последнее слово
				arguments = str_add(arguments, s);
			n = 0;
			s = realloc(s, n + 1);
			s[0] = 0;
		}
	}
	if (arguments !=NULL)
		Sh=make_shell(arguments,Sh);
	while (arguments != NULL)
	{
		printf("%s\n", arguments->str);
		arguments = arguments->next;
	}

	/*if (arguments != NULL)
	 {                 // переносим все аргументы в массив

	 background_pids = run_process(arguments, background_pids);
	 str_free(arguments);
	 arguments = NULL;
	 }

	 background_pids = check_background_processes(background_pids);
	 n = 0;
	 s = realloc(s, n + 1);
	 s[0] = 0;
	 fn_flag = 0;
	 }
	 }

	 free(s);

	 str_free(arguments);

	 kill_background_processes(background_pids);*/

	return 0;
}
