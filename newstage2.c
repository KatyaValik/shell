#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <dirent.h>
#include <sys/types.h>
#include <errno.h>

typedef struct node *list;
struct node {
	char* key;
	list next;
};
int n_arg;
struct info {
	list command;
	char* input;
	char* output;
	int background;
};

typedef struct nod *lis;
struct nod {
	pid_t pidfn;
	lis nex;
};
lis fnlis;

void del(pid_t pid) {
	if (fnlis->pidfn == pid) {
		lis Lfn = fnlis;
		fnlis = fnlis->nex;
		free(Lfn);
	} else {
		lis Lfnst = fnlis;
		while (Lfnst->nex->pidfn != pid) {
			Lfnst = Lfnst->nex;
		}
		lis Lfn = Lfnst->nex;
		Lfnst->nex = Lfn->nex;
		free(Lfn);
	}
}

void lfree(list L) {
	if (L->next == NULL) {
		free(L->key);
		free(L);
	} else {
		lfree(L->next);
		free(L->key);
		free(L);
	}
}

void my_cd(char* path) {
	int f;
	f = chdir(path);
	if (f == -1) {
		printf("no such file or directory\n");
	}
}

int main() {
	fnlis = NULL;
	lis Lfnst;
	int status;
	char c;
	pid_t pid;
	int n = 0;
	list L = NULL;
	list Lst;
	n_arg = 0;
	int fn_flag = 0;
	char *s = (char*) calloc(1, sizeof(char));
	char** arg = NULL;
	while ((c = getchar()) != EOF) {                     //читаю слова
		if (c != '\n') {
			if (c == '"') {
				if (fn_flag) {
					printf("problem with fn\n");
					return (1);
				}
				while ((c = getchar()) != '"') {
					if (c == EOF) {
						printf("\n");
						printf("problem\n");
						return 1;
					}
					s = realloc(s, strlen(s) + 1);
					s[n] = c;
					n++;
					s[n] = 0;
				}
			} else {
				if (c != ' ') {
					/*if (fn_flag) {
					 printf("problem with fn\n");
					 lfree(L);
					 L=NULL;
					 n_arg=0;
					 return(1);
					 }*/
					s = realloc(s, strlen(s) + 1);
					s[n] = c;
					n++;
					s[n] = 0;
					if (c == '&') {
						fn_flag = 1;
					}
				} else {
					list L1 = malloc(sizeof(list));
					L1->key = malloc(strlen(s));
					strcpy(L1->key, s);
					n_arg++;
					L1->next = NULL;
					Lst = L;
					if (L == NULL) {
						L = L1;
					} else {
						while (L->next != NULL) {
							L = L->next;
						}
						L->next = L1;
						L = Lst;
					}
					n = 0;
				}
			}
		} else {
			Lfnst = fnlis;                     // если встретили /n
			while (Lfnst != NULL)              //проверка фоновых процессов
			{
				if (waitpid(Lfnst->pidfn, &status, WNOHANG)) {
					if (WIFEXITED(status)) {
						int ex = WEXITSTATUS(status);
						printf("process with PID=%d finished with status %d\n", Lfnst->pidfn, ex);
						del(Lfnst->pidfn);
						lis Lf = Lfnst->nex;
					}

				}
				Lfnst = Lfnst->nex;
			}
			if (strlen(s) != 0) {                     // если не внесли в список последнее слово
				list L1 = malloc(sizeof(list));
				L1->key = malloc(strlen(s));
				strcpy(L1->key, s);
				n_arg++;
				L1->next = NULL;
				Lst = L;
				if (L == NULL) {
					L = L1;
				} else {
					while (L->next != NULL) {
						L = L->next;
					}
					L->next = L1;
					L = Lst;
				}
				n = 0;
			}
			if (L != NULL) {                            // переносим все аргументы в массив
				Lst = L;
				arg = realloc(arg, (n_arg + 1) * (sizeof(L->key)));
				for (int j = 0; j < n_arg; j++) {
					arg[j] = Lst->key;
					Lst = Lst->next;
				}
				arg[n_arg] = NULL;
				n = 0;
				if (fn_flag) {
					if (strcmp(arg[n_arg - 1], "&") == 0) {
						arg = realloc(arg, n_arg - 1);
						arg[n_arg] = NULL;
						pid = fork();
						if (!pid) {

							execvp(arg[0], arg);
							fprintf(stderr, "Error executing %s: %s\n", arg[0], strerror(errno));
							exit(1);
						}
						lis Lfn = malloc(sizeof(lis));
						Lfn->pidfn = pid;
						Lfn->nex = NULL;
						Lfnst = Lfn;
						if (fnlis == NULL) {
							fnlis = Lfn;
						} else {
							while (Lfnst->nex != NULL) {
								Lfnst = Lfnst->nex;
							}
							Lfnst->nex = Lfn;
						}
						printf("running the background process with PID=%d\n", pid);
						fn_flag = 0;
						lfree(L);
						L = NULL;
						n_arg = 0;

					} else {
						printf("problem with fn\n");
						lfree(L);
						L = NULL;
						n_arg = 0;
						fn_flag = 0;
					}
				} else if (strcmp(arg[0], "cd") == 0) {
					if (n_arg > 2) {
						printf("problem with cd\n");
						lfree(L);
						L = NULL;
						n_arg = 0;
						//return(1);
					} else {
						my_cd(arg[1]);
						lfree(L);
						L = NULL;
						n_arg = 0;
					}
				} else {
					if (!fork()) {
						execvp(arg[0], arg);
						exit(1);
					} else {
						wait(NULL);
						lfree(L);
						L = NULL;
						n_arg = 0;
					}
				}
			}
		}
	}
	free(s);
	int fl = 0;

	if (L == NULL)
		fl = 1;
	if (!fl)
		printf("\n");
}

