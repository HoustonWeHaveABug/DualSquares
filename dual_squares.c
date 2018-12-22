#include <stdio.h>
#include <stdlib.h>

#define ORDER_MIN 2
#define VALUE_X 1
#define VALUE_O 2
#define VALUE_ALL (VALUE_X+VALUE_O)
#define SYMBOL_X 'X'
#define SYMBOL_O 'O'

typedef struct {
	int forced_n;
	int singles_n;
}
score_t;

int sum_first_n(int);
int dual_squares(int, int, int, int, int);
void backup_row(int, int, int, int);
void backup_column(int, int, int, int);
void evaluate_cell(int, int, int, int, int, int, score_t *);
int choose_cell(int, int, int, int, int, int, int);
void check_row(int, int, int, int, score_t *);
void check_column(int, int, int, int, score_t *);
int check_corners(int, int *, int *, int *);
int check_singles(int, int);
void set_score(score_t *, int, int);
void add_score(score_t *, score_t *);
int compare_scores(score_t *, score_t *);
void restore_row(int, int, int, int);
void restore_column(int, int, int, int);

int order, singles_max, *active, *backup, *found;

int main(void) {
	int active_size, backup_size, i;
	if (scanf("%d%d", &order, &singles_max) != 2 || order < ORDER_MIN || singles_max < 0) {
		fprintf(stderr, "Invalid parameters\n");
		fflush(stderr);
		return EXIT_FAILURE;
	}
	singles_max++;
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
	found = malloc(sizeof(int)*(size_t)order);
	if (!found) {
		fprintf(stderr, "Could not allocate memory for found\n");
		fflush(stderr);
		free(backup);
		free(active);
		return EXIT_FAILURE;
	}
	for (i = 0; i < order; i++) {
		found[i] = singles_max;
	}
	dual_squares(1, 0, 0, 1, 0);
	fflush(stdout);
	free(found);
	free(backup);
	free(active);
	return EXIT_SUCCESS;
}

int sum_first_n(int n) {
	return n*(n+1)/2;
}

int dual_squares(int x, int y, int backup_start, int symmetric, int singles_n) {
	if (x == order) {
		int i;
		if (singles_n < singles_max) {
			singles_max = singles_n;
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
		return singles_max == 0;
	}
	if (active[y*order+x] == VALUE_X || active[y*order+x] == VALUE_O) {
		return choose_cell(x, y, backup_start, symmetric, singles_n, active[y*order+x], 0);
	}
	else if (active[y*order+x] == VALUE_ALL) {
		int choice1, choice2, r;
		score_t score_x, score_o, score1, score2;
		if (x <= y) {
			backup_row(y, x, y, backup_start);
		}
		else {
			backup_column(x, y, x-1, backup_start);
		}
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
			choice1 = VALUE_X;
			score1 = score_x;
			choice2 = VALUE_O;
			score2 = score_o;
		}
		else {
			choice1 = VALUE_O;
			score1 = score_o;
			choice2 = VALUE_X;
			score2 = score_x;
		}
		if (score1.singles_n < singles_max) {
			if (x <= y) {
				restore_row(y, x, y, backup_start);
			}
			else {
				restore_column(x, y, x-1, backup_start);
			}
			active[y*order+x] = choice1;
			r = choose_cell(x, y, backup_start, symmetric, singles_n, choice1, 1);
			if (score2.singles_n < singles_max && !r) {
				if (x <= y) {
					restore_row(y, x, y, backup_start);
				}
				else {
					restore_column(x, y, x-1, backup_start);
				}
				active[y*order+x] = choice2;
				r = choose_cell(x, y, backup_start, symmetric, singles_n, choice2, 1);
			}
		}
		else {
			r = 0;
		}
		if (x <= y) {
			restore_row(y, x, y, backup_start);
		}
		else {
			restore_column(x, y, x-1, backup_start);
		}
		return r;
	}
	else {
		fprintf(stderr, "This should never happen\n");
		fflush(stderr);
		return 0;
	}
}

void backup_row(int y, int x_min, int x_max, int backup_idx) {
	int i;
	for (i = x_min; i <= x_max; i++) {
		backup[backup_idx++] = active[y*order+i];
	}
}

void backup_column(int x, int y_min, int y_max, int backup_idx) {
	int i;
	for (i = y_min; i <= y_max; i++) {
		backup[backup_idx++] = active[i*order+x];
	}
}

void evaluate_cell(int x, int y, int backup_start, int symmetric, int singles_n, int choice, score_t *total) {
	if (x < y) {
		int forced_count, x_ref, backup_idx;
		score_t score;
		if (symmetric) {
			if (choice < active[x*order+y]) {
				set_score(total, 0, singles_max);
				return;
			}
		}
		set_score(total, 0, singles_n);
		check_row(y, x, y, choice, &score);
		add_score(total, &score);
		forced_count = score.forced_n;
		for (x_ref = x+1, backup_idx = backup_start+1; x_ref < y && check_singles(y, total->singles_n) && forced_count > 0; x_ref++, backup_idx++) {
			if (active[y*order+x_ref] < backup[backup_idx]) {
				forced_count--;
			}
			if (active[y*order+x_ref] < VALUE_ALL) {
				check_row(y, x_ref, y, active[y*order+x_ref], &score);
				add_score(total, &score);
				forced_count += score.forced_n;
			}
		}
		if (!check_singles(y, total->singles_n)) {
			set_score(total, 0, singles_max);
		}
	}
	else if (x-1 > y) {
		int forced_count, y_ref, backup_idx;
		score_t score;
		set_score(total, 0, singles_n);
		check_column(x, y, x-1, choice, &score);
		add_score(total, &score);
		forced_count = score.forced_n;
		for (y_ref = y+1, backup_idx = backup_start+1; y_ref < x-1 && check_singles(x, total->singles_n) && forced_count > 0; y_ref++, backup_idx++) {
			if (active[y_ref*order+x] < backup[backup_idx]) {
				forced_count--;
			}
			if (active[y_ref*order+x] < VALUE_ALL) {
				check_column(x, y_ref, x-1, active[y_ref*order+x], &score);
				add_score(total, &score);
				forced_count += score.forced_n;
			}
		}
		if (!check_singles(x, total->singles_n)) {
			set_score(total, 0, singles_max);
		}
	}
	else {
		if (check_singles(x, singles_n)) {
			set_score(total, 0, singles_n);
		}
		else {
			set_score(total, 0, singles_max);
		}
	}
}

int choose_cell(int x, int y, int backup_start, int symmetric, int singles_n1, int choice, int lookahead) {
	if (x < y) {
		int singles_n2 = 0;
		score_t score;
		if (symmetric) {
			if (choice > active[x*order+y]) {
				symmetric = 0;
			}
			else if (choice < active[x*order+y]) {
				return 0;
			}
		}
		check_row(y, x, y, choice, &score);
		singles_n1 += score.singles_n;
		singles_n2 = 0;
		if (lookahead) {
			int forced_count = score.forced_n, x_ref, backup_idx;
			for (x_ref = x+1, backup_idx = backup_start+1; x_ref < y && check_singles(y, singles_n1+singles_n2) && forced_count > 0; x_ref++, backup_idx++) {
				if (active[y*order+x_ref] < backup[backup_idx]) {
					forced_count--;
				}
				if (active[y*order+x_ref] < VALUE_ALL) {
					check_row(y, x_ref, y, active[y*order+x_ref], &score);
					singles_n2 += score.singles_n;
					forced_count += score.forced_n;
				}
			}
		}
		return check_singles(y, singles_n1+singles_n2) && dual_squares(y, x+1, backup_start+y-x+1, symmetric, singles_n1);
	}
	else if (x-1 > y) {
		int singles_n2 = 0;
		score_t score;
		check_column(x, y, x-1, choice, &score);
		singles_n1 += score.singles_n;
		singles_n2 = 0;
		if (lookahead) {
			int forced_count = score.forced_n, y_ref, backup_idx;
			for (y_ref = y+1, backup_idx = backup_start+1; y_ref < x-1 && check_singles(x, singles_n1+singles_n2) && forced_count > 0; y_ref++, backup_idx++) {
				if (active[y_ref*order+x] < backup[backup_idx]) {
					forced_count--;
				}
				if (active[y_ref*order+x] < VALUE_ALL) {
					check_column(x, y_ref, x-1, active[y_ref*order+x], &score);
					singles_n2 += score.singles_n;
					forced_count += score.forced_n;
				}
			}
		}
		return check_singles(x, singles_n1+singles_n2) && dual_squares(y, x, backup_start+x-y, symmetric, singles_n1);
	}
	else if (x > y) {
		return check_singles(x, singles_n1) && dual_squares(y, x, backup_start+x-y, symmetric, singles_n1);
	}
	else {
		if (singles_n1 < found[x]) {
			found[x] = singles_n1;
			printf("N = %d S = %d\n", x+1, singles_n1);
			fflush(stdout);
		}
		return check_singles(x, singles_n1) && dual_squares(x+1, 0, backup_start+1, symmetric, singles_n1);
	}
}

void check_row(int y_ref, int x_ref, int x_max, int value_ref, score_t *total) {
	int *corner1 = active+y_ref*order+x_ref, *corner2 = corner1-order, *corner3 = corner2+1, *corner4 = corner3+order, i;
	set_score(total, 0, 0);
	for (i = x_ref+1; i <= x_max; i++, corner2 -= order, corner3 -= order-1, corner4++) {
		score_t score;
		int r = check_corners(value_ref, corner2, corner3, corner4);
		set_score(&score, r & 1, (r & 2) >> 1);
		add_score(total, &score);
	}
}

void check_column(int x_ref, int y_ref, int y_max, int value_ref, score_t *total) {
	int *corner1 = active+y_ref*order+x_ref, *corner2 = corner1-1, *corner3 = corner2+order, *corner4 = corner3+1, i;
	set_score(total, 0, 0);
	for (i = y_ref+1; i <= y_max; i++, corner2--, corner3 += order-1, corner4 += order) {
		score_t score;
		int r = check_corners(value_ref, corner2, corner3, corner4);
		set_score(&score, r & 1, (r & 2) >> 1);
		add_score(total, &score);
	}
}

int check_corners(int value_ref, int *corner2, int *corner3, int *corner4) {
	if (*corner2 == value_ref && *corner3 == value_ref && (*corner4 & value_ref)) {
		if (*corner4 == value_ref) {
			return 2;
		}
		*corner4 -= value_ref;
		return 1;
	}
	return 0;
}

int check_singles(int x, int singles_n) {
	return singles_n < singles_max && (singles_max == 1 || (double)singles_n/(singles_max-1) <= (double)x*x/(order-1)/(order-1));
}

void set_score(score_t *score, int forced_n, int singles_n) {
	score->forced_n = forced_n;
	score->singles_n = singles_n;
}

void add_score(score_t *total, score_t *score) {
	total->forced_n += score->forced_n;
	total->singles_n += score->singles_n;
}

int compare_scores(score_t *score_a, score_t *score_b) {
	if (score_a->singles_n != score_b->singles_n) {
		return score_a->singles_n-score_b->singles_n;
	}
	return score_b->forced_n-score_a->forced_n;
}

void restore_row(int y, int x_min, int x_max, int backup_idx) {
	int i;
	for (i = x_min; i <= x_max; i++) {
		active[y*order+i] = backup[backup_idx++];
	}
}

void restore_column(int x, int y_min, int y_max, int backup_idx) {
	int i;
	for (i = y_min; i <= y_max; i++) {
		active[i*order+x] = backup[backup_idx++];
	}
}
