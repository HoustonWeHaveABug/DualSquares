#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define ORDER_MIN 2
#define VALUE_X 1
#define VALUE_O 2
#define VALUE_ALL (VALUE_X+VALUE_O)
#define SYMBOL_X 'X'
#define SYMBOL_O 'O'

typedef struct {
	int changes_n;
	int singles_n;
}
score_t;

int sum_first_n(int);
void dual_squares(int, int, int, int, int);
void backup_row(int, int, int, int);
void backup_column(int, int, int, int);
void evaluate_cell(int, int, int, int, int, int, score_t *);
void choose_cell(int, int, int, int, int, int, int);
void check_east_corner4(int, int, int, score_t *);
void check_south_corner4(int, int, int, score_t *);
int check_corner4(int *, int *, int *, int *);
void check_east_corner1(int, int, int, score_t *);
void check_south_corner1(int, int, int, score_t *);
int check_corner1(int *, int *, int *, int *);
void set_score(score_t *, int, int);
void add_score(score_t *, score_t *);
int compare_scores(score_t *, score_t *);
void restore_row(int, int, int, int);
void restore_column(int, int, int, int);

int order, singles_max, counter_period, *active, *backup, counter1, counter2, x_switch, y_switch, flag_switch;
unsigned time0;

int main(void) {
	int active_size, backup_size, i;
	if (scanf("%d%d%d", &order, &singles_max, &counter_period) != 3 || order < ORDER_MIN || singles_max < 0 || counter_period < 1) {
		fprintf(stderr, "Invalid parameters\n");
		fflush(stderr);
		return EXIT_FAILURE;
	}
	active_size = order*order;
	active = malloc(sizeof(int)*(size_t)active_size);
	if (!active) {
		fprintf(stderr, "Could not allocate memory for active\n");
		fflush(stderr);
		return EXIT_FAILURE;
	}
	active[0] = VALUE_X;
	for (i = 1; i < active_size; i++) {
		active[i] = VALUE_ALL;
	}
	backup_size = 1;
	for (i = 2; i < order; i++) {
		backup_size += sum_first_n(i)*2;
	}
	backup_size += sum_first_n(order);
	backup = malloc(sizeof(int)*(size_t)backup_size);
	if (!backup) {
		fprintf(stderr, "Could not allocate memory for backup\n");
		fflush(stderr);
		free(active);
		return EXIT_FAILURE;
	}
	counter1 = 0;
	counter2 = 0;
	time0 = (unsigned)time(NULL);
	if (singles_max == 0) {
		dual_squares(1, 0, 0, 1, 0);
	}
	else {
		x_switch = 0;
		y_switch = 0;
		flag_switch = 1;
		printf("x_switch %d y_switch %d\n", x_switch, y_switch);
		fflush(stdout);
		dual_squares(1, 0, 0, 1, 0);
		flag_switch = 0;
		x_switch = 1;
		y_switch = 0;
		while (x_switch < order) {
			printf("x_switch %d y_switch %d\n", x_switch, y_switch);
			fflush(stdout);
			dual_squares(1, 0, 0, 1, 0);
			if (x_switch < y_switch) {
				int tmp = x_switch;
				x_switch = y_switch;
				y_switch = tmp+1;
			}
			else if (x_switch > y_switch) {
				int tmp = x_switch;
				x_switch = y_switch;
				y_switch = tmp;
			}
			else {
				x_switch++;
				y_switch = 0;
			}
		}
	}
	printf("Solutions %d*%d+%d Time %us\n", counter2, counter_period, counter1, (unsigned)time(NULL)-time0);
	fflush(stdout);
	free(backup);
	free(active);
	return EXIT_SUCCESS;
}

int sum_first_n(int n) {
	return n*(n+1)/2;
}

void dual_squares(int x, int y, int backup_start, int symmetric, int singles_n) {
	if (x == order) {
		counter1++;
		if (counter1 == counter_period) {
			counter1 = 0;
			counter2++;
			printf("Solutions %d*%d Time %us\n", counter2, counter_period, (unsigned)time(NULL)-time0);
			fflush(stdout);
		}
		if (singles_n < singles_max || (counter1 == 1 && counter2 == 0)) {
			int i;
			printf("Singles %d\n", singles_n);
			fflush(stdout);
			if (singles_n < singles_max) {
				singles_max = singles_n;
			}
			for (i = 0; i < order; i++) {
				int j;
				if (active[i*order] == VALUE_X) {
					putchar(SYMBOL_X);
				}
				else if (active[i*order] == VALUE_O) {
					putchar(SYMBOL_O);
				}
				else {
					fprintf(stderr, "This should never happen\n");
					fflush(stderr);
				}
				for (j = 1; j < order; j++) {
					putchar(' ');
					if (active[i*order+j] == VALUE_X) {
						putchar(SYMBOL_X);
					}
					else if (active[i*order+j] == VALUE_O) {
						putchar(SYMBOL_O);
					}
					else {
						fprintf(stderr, "This should never happen\n");
						fflush(stderr);
					}
				}
				puts("");
			}
			fflush(stdout);
		}
		return;
	}
	if (singles_max == 0) {
		if (active[y*order+x] < VALUE_ALL) {
			choose_cell(x, y, backup_start, symmetric, singles_n, active[y*order+x], 0);
		}
		else if (active[y*order+x] == VALUE_ALL) {
			if (x <= y) {
				backup_row(y, x, y, backup_start);
			}
			else {
				backup_column(x, y, x-1, backup_start);
			}
			active[y*order+x] = VALUE_X;
			choose_cell(x, y, backup_start, symmetric, singles_n, VALUE_X, 1);
			if (x <= y) {
				restore_row(y, x, y, backup_start);
			}
			else {
				restore_column(x, y, x-1, backup_start);
			}
			active[y*order+x] = VALUE_O;
			choose_cell(x, y, backup_start, symmetric, singles_n, VALUE_O, 1);
			if (x <= y) {
				restore_row(y, x, y, backup_start);
			}
			else {
				restore_column(x, y, x-1, backup_start);
			}
		}
		else {
			fprintf(stderr, "This should never happen\n");
			fflush(stderr);
		}
	}
	else {
		if (active[y_switch*order+x_switch] == VALUE_X) {
			return;
		}
		if (active[y*order+x] < VALUE_ALL) {
			if (x == x_switch && y == y_switch) {
				flag_switch = 1;
				choose_cell(x, y, backup_start, symmetric, singles_n, active[y*order+x], 0);
				flag_switch = 0;
			}
			else {
				choose_cell(x, y, backup_start, symmetric, singles_n, active[y*order+x], 0);
			}
		}
		else if (active[y*order+x] == VALUE_ALL) {
			if (x <= y) {
				backup_row(y, x, y, backup_start);
			}
			else {
				backup_column(x, y, x-1, backup_start);
			}
			if (x == x_switch && y == y_switch) {
				flag_switch = 1;
				active[y*order+x] = VALUE_O;
				choose_cell(x, y, backup_start, symmetric, singles_n, VALUE_O, 1);
				flag_switch = 0;
			}
			else {
				if (flag_switch) {
					int choice;
					score_t score_x, score_o, score;
					active[y*order+x] = VALUE_X;
					evaluate_cell(x, y, backup_start, symmetric, singles_n, VALUE_X, &score_x);
					if (x <= y) {
						restore_row(y, x, y, backup_start);
					}
					else {
						restore_column(x, y, x-1, backup_start);
					}
					active[y*order+x] = VALUE_O;
					evaluate_cell(x, y, backup_start, symmetric, singles_n, VALUE_O, &score_o);
					if (compare_scores(&score_x, &score_o) <= 0) {
						choice = VALUE_X;
						score = score_x;
					}
					else {
						choice = VALUE_O;
						score = score_o;
					}
					if (score.singles_n <= singles_max) {
						if (x <= y) {
							restore_row(y, x, y, backup_start);
						}
						else {
							restore_column(x, y, x-1, backup_start);
						}
						active[y*order+x] = choice;
						choose_cell(x, y, backup_start, symmetric, singles_n, choice, 1);
					}
				}
				else {
					active[y*order+x] = VALUE_X;
					choose_cell(x, y, backup_start, symmetric, singles_n, VALUE_X, 1);
					if (x <= y) {
						restore_row(y, x, y, backup_start);
					}
					else {
						restore_column(x, y, x-1, backup_start);
					}
					active[y*order+x] = VALUE_O;
					choose_cell(x, y, backup_start, symmetric, singles_n, VALUE_O, 1);
				}
			}
			if (x <= y) {
				restore_row(y, x, y, backup_start);
			}
			else {
				restore_column(x, y, x-1, backup_start);
			}
		}
		else {
			fprintf(stderr, "This should never happen\n");
			fflush(stderr);
		}
	}
}

void backup_row(int y, int x_min, int x_max, int backup_start) {
	for (; x_min <= x_max; x_min++) {
		backup[backup_start++] = active[y*order+x_min];
	}
}

void backup_column(int x, int y_min, int y_max, int backup_start) {
	for (; y_min <= y_max; y_min++) {
		backup[backup_start++] = active[y_min*order+x];
	}
}

void evaluate_cell(int x, int y, int backup_start, int symmetric, int singles_n, int choice, score_t *total) {
	if (x < y) {
		int changes_sum, x_ref, backup_idx;
		score_t score;
		if (symmetric) {
			if (choice < active[x*order+y]) {
				set_score(total, 0, singles_max+1);
				return;
			}
		}
		set_score(total, 0, singles_n);
		check_east_corner4(y, x, y, &score);
		changes_sum = score.changes_n;
		add_score(total, &score);
		for (x_ref = x+1, backup_idx = backup_start+1; x_ref < y && changes_sum > 0 && total->singles_n <= singles_max; x_ref++, backup_idx++) {
			if (active[y*order+x_ref] == VALUE_ALL) {
				check_east_corner1(y, x_ref, y, &score);
				changes_sum += score.changes_n;
				total->changes_n += score.changes_n;
			}
			if (active[y*order+x_ref] < backup[backup_idx]) {
				changes_sum--;
			}
			if (active[y*order+x_ref] < VALUE_ALL) {
				check_east_corner4(y, x_ref, y, &score);
				changes_sum += score.changes_n;
				add_score(total, &score);
			}
		}
		if (total->singles_n > singles_max) {
			set_score(total, 0, singles_max+1);
		}
	}
	else if (x-1 > y) {
		int changes_sum, y_ref, backup_idx;
		score_t score;
		set_score(total, 0, singles_n);
		check_south_corner4(x, y, x-1, &score);
		changes_sum = score.changes_n;
		add_score(total, &score);
		for (y_ref = y+1, backup_idx = backup_start+1; y_ref < x-1 && changes_sum > 0 && total->singles_n <= singles_max; y_ref++, backup_idx++) {
			if (active[y_ref*order+x] == VALUE_ALL) {
				check_south_corner1(x, y_ref, x-1, &score);
				changes_sum += score.changes_n;
				total->changes_n += score.changes_n;
			}
			if (active[y_ref*order+x] < backup[backup_idx]) {
				changes_sum--;
			}
			if (active[y_ref*order+x] < VALUE_ALL) {
				check_south_corner4(x, y_ref, x-1, &score);
				changes_sum += score.changes_n;
				add_score(total, &score);
			}
		}
		if (total->singles_n > singles_max) {
			set_score(total, 0, singles_max+1);
		}
	}
	else {
		set_score(total, 0, singles_n);
	}
}

void choose_cell(int x, int y, int backup_start, int symmetric, int singles_n1, int choice, int lookahead) {
	if (x < y) {
		int singles_n2;
		score_t score;
		if (symmetric) {
			if (choice > active[x*order+y]) {
				symmetric = 0;
			}
			else if (choice < active[x*order+y]) {
				return;
			}
		}
		check_east_corner4(y, x, y, &score);
		singles_n1 += score.singles_n;
		singles_n2 = 0;
		if (lookahead) {
			int changes_sum = score.changes_n, x_ref, backup_idx;
			for (x_ref = x+1, backup_idx = backup_start+1; x_ref < y && changes_sum > 0 && singles_n1+singles_n2 <= singles_max; x_ref++, backup_idx++) {
				if (active[y*order+x_ref] == VALUE_ALL) {
					check_east_corner1(y, x_ref, y, &score);
					changes_sum += score.changes_n;
				}
				if (active[y*order+x_ref] < backup[backup_idx]) {
					changes_sum--;
				}
				if (active[y*order+x_ref] < VALUE_ALL) {
					check_east_corner4(y, x_ref, y, &score);
					changes_sum += score.changes_n;
					singles_n2 += score.singles_n;
				}
			}
		}
		if (singles_n1+singles_n2 <= singles_max) {
			dual_squares(y, x+1, backup_start+y-x+1, symmetric, singles_n1);
		}
	}
	else if (x-1 > y) {
		int singles_n2;
		score_t score;
		check_south_corner4(x, y, x-1, &score);
		singles_n1 += score.singles_n;
		singles_n2 = 0;
		if (lookahead) {
			int changes_sum = score.changes_n, y_ref, backup_idx;
			for (y_ref = y+1, backup_idx = backup_start+1; y_ref < x-1 && changes_sum > 0 && singles_n1+singles_n2 <= singles_max; y_ref++, backup_idx++) {
				if (active[y_ref*order+x] == VALUE_ALL) {
					check_south_corner1(x, y_ref, x-1, &score);
					changes_sum += score.changes_n;
				}
				if (active[y_ref*order+x] < backup[backup_idx]) {
					changes_sum--;
				}
				if (active[y_ref*order+x] < VALUE_ALL) {
					check_south_corner4(x, y_ref, x-1, &score);
					changes_sum += score.changes_n;
					singles_n2 += score.singles_n;
				}
			}
		}
		if (singles_n1+singles_n2 <= singles_max) {
			dual_squares(y, x, backup_start+x-y, symmetric, singles_n1);
		}
	}
	else if (x > y) {
		dual_squares(y, x, backup_start+x-y, symmetric, singles_n1);
	}
	else {
		dual_squares(x+1, 0, backup_start+1, symmetric, singles_n1);
	}
}

void check_east_corner4(int y_ref, int x_min, int x_max, score_t *total) {
	int *corner1 = active+y_ref*order+x_min, *corner2 = corner1-order, *corner3 = corner2+1, *corner4 = corner3+order;
	set_score(total, 0, 0);
	for (x_min++; x_min <= x_max; x_min++, corner2 -= order, corner3 -= order-1, corner4++) {
		int r = check_corner4(corner1, corner2, corner3, corner4);
		score_t score;
		set_score(&score, r & 1, (r & 2) >> 1);
		add_score(total, &score);
	}
}

void check_south_corner4(int x_ref, int y_min, int y_max, score_t *total) {
	int *corner1 = active+y_min*order+x_ref, *corner2 = corner1-1, *corner3 = corner2+order, *corner4 = corner3+1;
	set_score(total, 0, 0);
	for (y_min++; y_min <= y_max; y_min++, corner2--, corner3 += order-1, corner4 += order) {
		int r = check_corner4(corner1, corner2, corner3, corner4);
		score_t score;
		set_score(&score, r & 1, (r & 2) >> 1);
		add_score(total, &score);
	}
}

int check_corner4(int *corner1, int *corner2, int *corner3, int *corner4) {
	if (*corner1 == *corner2 && *corner2 == *corner3 && (*corner4 & *corner1)) {
		if (*corner4 == *corner1) {
			return 2;
		}
		*corner4 -= *corner1;
		return 1;
	}
	return 0;
}

void check_east_corner1(int y_ref, int x_min, int x_max, score_t *total) {
	int *corner1 = active+y_ref*order+x_min, *corner2 = corner1-order, *corner3 = corner2+1, *corner4 = corner3+order;
	set_score(total, 0, 0);
	for (x_min++; x_min <= x_max && total->changes_n == 0; x_min++, corner2 -= order, corner3 -= order-1, corner4++) {
		int r = check_corner1(corner1, corner2, corner3, corner4);
		score_t score;
		set_score(&score, r & 1, (r & 2) >> 1);
		add_score(total, &score);
	}
}

void check_south_corner1(int x_ref, int y_min, int y_max, score_t *total) {
	int *corner1 = active+y_min*order+x_ref, *corner2 = corner1-1, *corner3 = corner2+order, *corner4 = corner3+1;
	set_score(total, 0, 0);
	for (y_min++; y_min <= y_max && total->changes_n == 0; y_min++, corner2--, corner3 += order-1, corner4 += order) {
		int r = check_corner1(corner1, corner2, corner3, corner4);
		score_t score;
		set_score(&score, r & 1, (r & 2) >> 1);
		add_score(total, &score);
	}
}

int check_corner1(int *corner1, int *corner2, int *corner3, int *corner4) {
	if ((*corner1 & *corner2) && *corner2 == *corner3 && *corner3 == *corner4) {
		if (*corner1 == *corner2) {
			return 2;
		}
		*corner1 -= *corner2;
		return 1;
	}
	return 0;
}

void set_score(score_t *score, int changes_n, int singles_n) {
	score->changes_n = changes_n;
	score->singles_n = singles_n;
}

void add_score(score_t *total, score_t *score) {
	total->changes_n += score->changes_n;
	total->singles_n += score->singles_n;
}

int compare_scores(score_t *score_a, score_t *score_b) {
	if (score_a->singles_n != score_b->singles_n) {
		return score_a->singles_n-score_b->singles_n;
	}
	return score_a->changes_n-score_b->changes_n;
}

void restore_row(int y, int x_min, int x_max, int backup_start) {
	for (; x_min <= x_max; x_min++) {
		active[y*order+x_min] = backup[backup_start++];
	}
}

void restore_column(int x, int y_min, int y_max, int backup_start) {
	for (; y_min <= y_max; y_min++) {
		active[y_min*order+x] = backup[backup_start++];
	}
}
