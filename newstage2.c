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
struct one_command
{
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

/*добавляет элемент списка*/
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

/*печать списка*/
void print_list(str_list L)
{
	str_list Lst = L;
	while (L != NULL)
	{
		printf("%s  ", L->str);
		L = L->next;
	}
	L = Lst;
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

/*удаление структуры shell*/
void shell_free(shell Sh)
{
	if (Sh == NULL)
		return;
	shell_free(Sh->next_command);
	str_free(Sh->comand);
	free(Sh->input);
	free(Sh->output);
	free(Sh->output_add);
}

/*разбор строки на слова*/
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
				memset(str1, 0, strlen(str) + 1);
				position += strlen(special_symbols[i]);
				L = str_add(L, special_symbols[i]);
				is_special = 1;
				break;
			}
		}

		if (is_special)
			continue;

		if ((str[position] == ' ') || (str[position] == '\n'))
		{
			if (n > 0)
				L = str_add(L, str1);
			n = 0;
			memset(str1, 0, strlen(str) + 1);
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

/*создание структуры для выполнения команд*/
shell make_shell(str_list L)
{
	str_list Lst = L;
	shell Sh = calloc(1, sizeof(struct one_command));
	shell Sh_next;
	shell curr_cmd = Sh;
	int str_size;
	while (L != NULL)
	{
		if (strcmp(L->str, ">") == 0)
		{
			str_size = strlen(L->next->str) + 1;
			curr_cmd->output = malloc(sizeof(char) * str_size);
			memcpy(curr_cmd->output, L->next->str, str_size);
			L = L->next->next;
		}
		else if (strcmp(L->str, "<") == 0)
		{
			str_size = strlen(L->next->str) + 1;
			curr_cmd->input = malloc(str_size);
			memcpy(curr_cmd->input, L->next->str, str_size);
			L = L->next->next;
		}
		else if (strcmp(L->str, ">>") == 0)
		{
			str_size = strlen(L->next->str) + 1;
			curr_cmd->output_add = malloc(str_size);
			memcpy(curr_cmd->output_add, L->next->str, str_size);
			L = L->next->next;
		}
		else if (strcmp(L->str, "&") == 0)
		{
			//curr_cmd->fn=malloc(sizeof(int));
			curr_cmd->fn = 1;
			L = L->next;
		}
		else if (strcmp(L->str, "|") == 0)
		{
			Sh_next = calloc(1, sizeof(struct one_command));
			curr_cmd->next_command = Sh_next;
			curr_cmd = curr_cmd->next_command;
			L = L->next;
		}
		else
		{
			curr_cmd->comand = str_add(curr_cmd->comand, L->str);
			L = L->next;
		}
	}
	str_free(L);
	return Sh;
}

/*печатает структуры shell*/
void print_shell(shell Sh)
{
	shell Sh_start = Sh;
	while (Sh != NULL)
	{
		print_list(Sh->comand);
		printf("\n");
		if (Sh->input != NULL)
			printf("%s\n", Sh->input);
		if (Sh->output != NULL)
			printf("%s\n", Sh->output);
		if (Sh->output_add != NULL)
			printf("%s\n", Sh->output_add);
		printf("%d\n", Sh->fn);
		Sh = Sh->next_command;
	}
	Sh = Sh_start;
	shell_free(Sh_start);
}

/*счаитаем количество команд конвейера*/
int comand_count(shell Sh)
{
	shell Sh_start;
	int count = 0;
	while (Sh != NULL)
	{
		count++;
		Sh = Sh->next_command;
	}
	return count;
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

/*проверка завершенных фоновых процессов*/
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

/*выполнение команды старое
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
 }*/

/*выполнение команды новое*/
pid_list run_process(shell Sh, pid_list pids)
{
	char** args;
	int nargs = str_size(Sh->comand);
	int background = 0;
	pid_t pid;

	if (check_cd(Sh->comand))
		return pids;

	args = malloc((nargs + 1) * sizeof(char*));
	nargs = 0;
	while (Sh->comand != NULL)
	{
		args[nargs] = Sh->comand->str;
		nargs++;
		Sh->comand = Sh->comand->next;
	}

	if (Sh->fn)
		background = 1;

	args[nargs] = NULL;

	pid = fork();
	if (!pid)
	{
		//if ((Sh->input!=NULL)||(Sh->output!=NULL)||(Sh->output_add!=NULL)){
		//	redir(Sh);
		//}
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

/*прибить фоновые процессы*/
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

/*конвейер*/
pid_list conv(shell Sh, pid_list background_pids)
{
	int count = comand_count(Sh);
	int fd[4];
	int *pred = fd;
	int *next = &fd[2];
	pid_t pid;
	if (count == 1)
	{
		background_pids = run_process(Sh, background_pids);
		return background_pids;

	}
	for (int i = 1; i <= count; i++)
	{
		if (i != count)
			pipe(next);
		pid = fork();
		if (!pid)
		{
			if (i != 1)
			{
				close(pred[1]);
				dup2(pred[0], 0);
				close(pred[0]); //????
			}
			if (i != count)
			{
				close(next[0]);
				dup2(next[1], 1);
				close(next[1]);
			}
			background_pids = run_process(Sh, background_pids);
			exit(1);
		}
		else
		{ //perent
			if (i != 1)
			{
				close(pred[0]);
				close(next[1]);
				int *tmp = pred;
				pred = next;
				next = tmp;
			}
			if (i != count)
				Sh = Sh->next_command;
		}
	}
	for (int i = 1; i <= count; i++)
	{
		wait(NULL);
	}
	return background_pids;
}

/*перенаправление ввода вывода*/
/*void redir(shell Sh) {
 int fd[2];
 pipe(fd);
 if (Sh->input!=NULL){
 dup2(fd[1],1);
 }
 }*/

int main()
{
	shell Sh = NULL;
	pid_list background_pids = NULL;
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
			background_pids = check_background_processes(background_pids);
			if (make_arguments(&arguments, str) == 0)
			{
				free(str);
				str = (char*) calloc(1, sizeof(char));
				if (arguments != NULL)
				{
					Sh = make_shell(arguments);
					background_pids = conv(Sh, background_pids);
					//print_shell(Sh);
					str_free(arguments);
					arguments = NULL;
					//shell_free(Sh);
				}
			}
		}
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

	 free(s);

	 str_free(arguments);*/

	kill_background_processes(background_pids);

	return 0;
}
