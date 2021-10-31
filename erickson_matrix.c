#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <limits.h>

#define COLORS_MIN 2
#define BITS_N 8
#define ORDER_MIN 2
#define SIZE_T_MAX (size_t)-1
#define ROT_CW_90 1
#define ROT_CW_180 2
#define ROT_CW_270 4
#define REF_AXIS_H 8
#define REF_AXIS_V 16
#define REF_DIAG_UB 32
#define REF_DIAG_BU 64

typedef struct {
	int pos;
	int cell;
}
backup_t;

typedef enum {
	NORTH,
	WEST,
	SOUTH,
	EAST
}
direction_t;

typedef struct {
	int changes_n;
	int singles_n;
}
score_t;

void erickson_matrix(int, int, int *, int, direction_t, int, int, int);
void print_cell(int);
void evaluate_cell(int, int, direction_t, int, int, score_t *);
void choose_cell(int, int, int *, int, direction_t, int, int, int, int, int);
void check_column(int, int, int, int *, score_t *);
void check_row(int, int, int, int *, score_t *);
int check_square(int *, int, int, int);
void set_backup(backup_t *, int, int);
void set_score(score_t *, int, int);
void add_score(score_t *, int, int);
int compare_scores(score_t *, score_t *);
void restore_row(int, int, int);
void restore_column(int, int, int);
int set_choices(int, int *);
int unique_choice(int);

int colors_n, order, singles_max, singles_switch, format_len, order_odd, *cells;
unsigned time0;
backup_t *backups;
score_t *scores;

int main(void) {
	int colors_div, cells_size, x_first, y_first, cells_first, colors_all, colors_idx, cells_idx, choices_all_size, *choices_all, backups_size;
	if (scanf("%d%d%d%d", &colors_n, &order, &singles_max, &singles_switch) != 4 || colors_n < COLORS_MIN || (size_t)(colors_n+1) >= sizeof(int)*BITS_N || order < ORDER_MIN || order > INT_MAX-1 || singles_max < 0 || singles_max > INT_MAX-1 || singles_switch < 0 || singles_switch > singles_max) {
		fprintf(stderr, "Invalid parameters\n");
		fflush(stderr);
		return EXIT_FAILURE;
	}
	for (colors_div = (colors_n-1)/10, format_len = 1; colors_div > 0; colors_div /= 10, format_len++);
	if (order%2 == 0) {
		order_odd = order+1;
	}
	else {
		order_odd = order;
	}
	if (order_odd > INT_MAX/order_odd || (size_t)order_odd > SIZE_T_MAX/(size_t)order_odd) {
		fprintf(stderr, "Could not set cells_size\n");
		fflush(stderr);
		return EXIT_FAILURE;
	}
	cells_size = order_odd*order_odd;
	if ((size_t)cells_size > SIZE_T_MAX/sizeof(int)) {
		fprintf(stderr, "Could not set cells size\n");
		fflush(stderr);
		return EXIT_FAILURE;
	}
	cells = malloc(sizeof(int)*(size_t)cells_size);
	if (!cells) {
		fprintf(stderr, "Could not allocate memory for cells\n");
		fflush(stderr);
		return EXIT_FAILURE;
	}
	x_first = order_odd/2;
	y_first = order_odd/2;
	cells_first = y_first*order_odd+x_first;
	colors_all = 1;
	for (colors_idx = 1; colors_idx < colors_n; colors_idx++) {
		colors_all += 1 << colors_idx;
	}
	for (cells_idx = 0; cells_idx < cells_first; cells_idx++) {
		cells[cells_idx] = colors_all;
	}
	cells[cells_idx] = 1;
	for (cells_idx++; cells_idx < cells_size; cells_idx++) {
		cells[cells_idx] = colors_all;
	}
	if (cells_size > INT_MAX/colors_n || (size_t)cells_size > SIZE_T_MAX/(size_t)colors_n) {
		fprintf(stderr, "Could not set choices_all_size\n");
		fflush(stderr);
		free(cells);
		return EXIT_FAILURE;
	}
	choices_all_size = colors_n*cells_size;
	if ((size_t)choices_all_size > SIZE_T_MAX/sizeof(int)) {
		fprintf(stderr, "Could not set choices_all size\n");
		fflush(stderr);
		free(cells);
		return EXIT_FAILURE;
	}
	choices_all = malloc(sizeof(int)*(size_t)choices_all_size);
	if (!choices_all) {
		fprintf(stderr, "Could not allocate memory for choices_all\n");
		fflush(stderr);
		free(cells);
		return EXIT_FAILURE;
	}
	backups_size = (colors_n-1)*cells_size;
	if ((size_t)backups_size > SIZE_T_MAX/sizeof(backup_t)) {
		fprintf(stderr, "Could not set backups size\n");
		fflush(stderr);
		free(choices_all);
		free(cells);
		return EXIT_FAILURE;
	}
	backups = malloc(sizeof(backup_t)*(size_t)backups_size);
	if (!backups) {
		fprintf(stderr, "Could not allocate memory for backups\n");
		fflush(stderr);
		free(choices_all);
		free(cells);
		return EXIT_FAILURE;
	}
	scores = malloc(sizeof(score_t)*(size_t)colors_n);
	if (!scores) {
		fprintf(stderr, "Could not allocate memory for scores\n");
		fflush(stderr);
		free(backups);
		free(choices_all);
		free(cells);
		return EXIT_FAILURE;
	}
	singles_max++;
	singles_switch++;
	time0 = (unsigned)time(NULL);
	erickson_matrix(x_first, y_first-1, choices_all, ROT_CW_90+ROT_CW_180+ROT_CW_270+REF_AXIS_H+REF_AXIS_V+REF_DIAG_UB+REF_DIAG_BU, WEST, 0, 2, 0);
	printf("Total time %us\n", (unsigned)time(NULL)-time0);
	fflush(stdout);
	free(scores);
	free(backups);
	free(choices_all);
	free(cells);
	return EXIT_SUCCESS;
}

void erickson_matrix(int x, int y, int *choices, int symmetric, direction_t direction, int backups_lo, int colors_max, int singles_n) {
	int choices_n;
	if (y == -1 || y == order) {
		int y_cur;
		singles_max = singles_n;
		if (singles_switch > singles_max) {
			singles_switch = singles_max;
		}
		printf("Singles %d Time %us\n", singles_max, (unsigned)time(NULL)-time0);
		for (y_cur = 0; y_cur < order; y_cur++) {
			int x_cur;
			print_cell(cells[y_cur*order_odd]);
			for (x_cur = 1; x_cur < order; x_cur++) {
				putchar(' ');
				print_cell(cells[y_cur*order_odd+x_cur]);
			}
			puts("");
		}
		fflush(stdout);
		return;
	}
	choices_n = set_choices(cells[y*order_odd+x], choices);
	if (choices_n == 1) {
		if (choices[0] == colors_max-1 && colors_max < colors_n) {
			choose_cell(x, y, choices, symmetric, direction, backups_lo, colors_max+1, singles_n, cells[y*order_odd+x], 0);
		}
		else {
			choose_cell(x, y, choices, symmetric, direction, backups_lo, colors_max, singles_n, cells[y*order_odd+x], 0);
		}
	}
	else {
		int choices_hi, choices_idx1;
		switch (direction) {
			case NORTH:
			case SOUTH:
				set_backup(backups+backups_lo, y, cells[y*order_odd+x]);
				break;
			case WEST:
			case EAST:
				set_backup(backups+backups_lo, x, cells[y*order_odd+x]);
				break;
			default:
				fprintf(stderr, "This should never happen\n");
				fflush(stderr);
				return;
		}
		for (choices_hi = 0; choices_hi < choices_n && choices[choices_hi] < colors_max; choices_hi++) {
			cells[y*order_odd+x] = 1 << choices[choices_hi];
			evaluate_cell(x, y, direction, backups_lo, singles_n, scores+choices_hi);
		}
		for (choices_idx1 = 1; choices_idx1 < choices_hi; choices_idx1++) {
			int choice = choices[choices_idx1], choices_idx2 = choices_idx1;
			score_t score = scores[choices_idx1];
			while (choices_idx2 > 0 && compare_scores(scores+choices_idx2-1, &score) > 0) {
				choices[choices_idx2] = choices[choices_idx2-1];
				scores[choices_idx2] = scores[choices_idx2-1];
				choices_idx2--;
			}
			choices[choices_idx2] = choice;
			scores[choices_idx2] = score;
		}
		while (choices_hi > 0 && scores[choices_hi-1].singles_n >= singles_max) {
			choices_hi--;
		}
		for (choices_idx1 = 0; choices_idx1 < choices_hi; choices_idx1++) {
			cells[y*order_odd+x] = 1 << choices[choices_idx1];
			if (choices[choices_idx1] == colors_max-1 && colors_max < colors_n) {
				choose_cell(x, y, choices, symmetric, direction, backups_lo, colors_max+1, singles_n, cells[y*order_odd+x], 1);
			}
			else {
				choose_cell(x, y, choices, symmetric, direction, backups_lo, colors_max, singles_n, cells[y*order_odd+x], 1);
			}
			if (singles_n >= singles_switch) {
				break;
			}
		}
		switch (direction) {
			case NORTH:
			case SOUTH:
				restore_column(x, backups_lo, backups_lo+1);
				break;
			case WEST:
			case EAST:
				restore_row(y, backups_lo, backups_lo+1);
				break;
			default:
				fprintf(stderr, "This should never happen\n");
				fflush(stderr);
		}
	}
}

void print_cell(int cell) {
	int choice = unique_choice(cell);
	if (choice < colors_n) {
		printf("%0*d", format_len, choice);
	}
	else {
		fprintf(stderr, "This should never happen\n");
		fflush(stderr);
	}
}

void evaluate_cell(int x, int y, direction_t direction, int backups_lo, int singles_n, score_t *total) {
	switch (direction) {
		int backups_hi, y_min, ref;
		score_t score;
		case NORTH:
			backups_hi = backups_lo+1;
			set_score(total, 0, singles_n);
			if (x == order-1) {
				y_min = 0;
			}
			else {
				y_min = order-2-x;
			}
			for (ref = y-1; ref >= y_min && total->singles_n < singles_max; ref--) {
				check_column(x, ref, backups_lo, &backups_hi, &score);
				add_score(total, score.changes_n, score.singles_n);
			}
			restore_column(x, backups_lo+1, backups_hi);
			break;
		case WEST:
			backups_hi = backups_lo+1;
			set_score(total, 0, singles_n);
			for (ref = x-1; ref >= y && total->singles_n < singles_max; ref--) {
				check_row(y, ref, backups_lo, &backups_hi, &score);
				add_score(total, score.changes_n, score.singles_n);
			}
			restore_row(y, backups_lo+1, backups_hi);
			break;
		case SOUTH:
			backups_hi = backups_lo+1;
			set_score(total, 0, singles_n);
			for (ref = y+1; ref <= order_odd-1-x && total->singles_n < singles_max; ref++) {
				check_column(x, ref, backups_lo, &backups_hi, &score);
				add_score(total, score.changes_n, score.singles_n);
			}
			restore_column(x, backups_lo+1, backups_hi);
			break;
		case EAST:
			backups_hi = backups_lo+1;
			set_score(total, 0, singles_n);
			for (ref = x+1; ref <= y && total->singles_n < singles_max; ref++) {
				check_row(y, ref, backups_lo, &backups_hi, &score);
				add_score(total, score.changes_n, score.singles_n);
			}
			restore_row(y, backups_lo+1, backups_hi);
			break;
		default:
			fprintf(stderr, "This should never happen\n");
			fflush(stderr);
	}
}

void choose_cell(int x, int y, int *choices, int symmetric, direction_t direction, int backups_lo, int colors_max, int singles_n, int choice, int lookahead) {
	direction_t direction_bak;
	if (symmetric) {
		int x_opp = order_odd-1-x, y_opp = order_odd-1-y, cell;
		if (symmetric & ROT_CW_90) {
			cell = cells[x*order_odd+y_opp];
			if (unique_choice(cell) < colors_n) {
				if (choice < cell) {
					return;
				}
				if (choice > cell) {
					symmetric -= ROT_CW_90;
				}
			}
		}
		if (symmetric & ROT_CW_180) {
			cell = cells[y_opp*order_odd+x_opp];
			if (unique_choice(cell) < colors_n) {
				if (choice < cell) {
					return;
				}
				if (choice > cell) {
					symmetric -= ROT_CW_180;
				}
			}
		}
		if (symmetric & ROT_CW_270) {
			cell = cells[x_opp*order_odd+y];
			if (unique_choice(cell) < colors_n) {
				if (choice < cell) {
					return;
				}
				if (choice > cell) {
					symmetric -= ROT_CW_270;
				}
			}
		}
		if (symmetric & REF_AXIS_H) {
			cell = cells[y_opp*order_odd+x];
			if (unique_choice(cell) < colors_n) {
				if (choice < cell) {
					return;
				}
				if (choice > cell) {
					symmetric -= REF_AXIS_H;
				}
			}
		}
		if (symmetric & REF_AXIS_V) {
			cell = cells[y*order_odd+x_opp];
			if (unique_choice(cell) < colors_n) {
				if (choice < cell) {
					return;
				}
				if (choice > cell) {
					symmetric -= REF_AXIS_V;
				}
			}
		}
		if (symmetric & REF_DIAG_UB) {
			cell = cells[x*order_odd+y];
			if (unique_choice(cell) < colors_n) {
				if (choice < cell) {
					return;
				}
				if (choice > cell) {
					symmetric -= REF_DIAG_UB;
				}
			}
		}
		if (symmetric & REF_DIAG_BU) {
			cell = cells[x_opp*order_odd+y_opp];
			if (unique_choice(cell) < colors_n) {
				if (choice < cell) {
					return;
				}
				if (choice > cell) {
					symmetric -= REF_DIAG_BU;
				}
			}
		}
	}
	direction_bak = direction;
	if (lookahead) {
		switch (direction_bak) {
			int backups_hi, ref;
			score_t score;
			case NORTH:
				backups_hi = backups_lo+1;
				for (ref = y-1; ref >= order_odd-1-x && singles_n < singles_max; ref--) {
					check_column(x, ref, backups_lo, &backups_hi, &score);
					singles_n += score.singles_n;
				}
				if (singles_n < singles_max) {
					y--;
					if (order_odd-2-x == y) {
						direction = WEST;
					}
					erickson_matrix(x, y, choices+colors_n, symmetric, direction, backups_hi, colors_max, singles_n);
				}
				restore_column(x, backups_lo+1, backups_hi);
				break;
			case WEST:
				backups_hi = backups_lo+1;
				for (ref = x-1; ref >= y && singles_n < singles_max; ref--) {
					check_row(y, ref, backups_lo, &backups_hi, &score);
					singles_n += score.singles_n;
				}
				if (singles_n < singles_max) {
					x--;
					if (x == y) {
						direction = SOUTH;
					}
					erickson_matrix(x, y, choices+colors_n, symmetric, direction, backups_hi, colors_max, singles_n);
				}
				restore_row(y, backups_lo+1, backups_hi);
				break;
			case SOUTH:
				backups_hi = backups_lo+1;
				for (ref = y+1; ref <= order_odd-1-x && singles_n < singles_max; ref++) {
					check_column(x, ref, backups_lo, &backups_hi, &score);
					singles_n += score.singles_n;
				}
				if (singles_n < singles_max) {
					y++;
					if (x == order_odd-1-y) {
						direction = EAST;
					}
					erickson_matrix(x, y, choices+colors_n, symmetric, direction, backups_hi, colors_max, singles_n);
				}
				restore_column(x, backups_lo+1, backups_hi);
				break;
			case EAST:
				backups_hi = backups_lo+1;
				for (ref = x+1; ref <= y && singles_n < singles_max; ref++) {
					check_row(y, ref, backups_lo, &backups_hi, &score);
					singles_n += score.singles_n;
				}
				if (singles_n < singles_max) {
					x++;
					if (x == y) {
						direction = NORTH;
					}
					erickson_matrix(x, y, choices+colors_n, symmetric, direction, backups_hi, colors_max, singles_n);
				}
				restore_row(y, backups_lo+1, backups_hi);
				break;
			default:
				fprintf(stderr, "This should never happen\n");
				fflush(stderr);
		}
	}
	else {
		switch (direction_bak) {
			case NORTH:
				y--;
				if (order_odd-2-x == y) {
					direction = WEST;
				}
				break;
			case WEST:
				x--;
				if (x == y) {
					direction = SOUTH;
				}
				break;
			case SOUTH:
				y++;
				if (x == order_odd-1-y) {
					direction = EAST;
				}
				break;
			case EAST:
				x++;
				if (x == y) {
					direction = NORTH;
				}
				break;
			default:
				fprintf(stderr, "This should never happen\n");
				fflush(stderr);
				return;
		}
		erickson_matrix(x, y, choices+colors_n, symmetric, direction, backups_lo, colors_max, singles_n);
	}
}

void check_column(int x_ref, int y_ref, int backups_lo, int *backups_hi, score_t *total) {
	int *corner1 = cells+y_ref*order_odd+x_ref;
	set_backup(backups+*backups_hi, y_ref, *corner1);
	set_score(total, 0, 0);
	while (backups_lo < *backups_hi) {
		int y_check = backups[backups_lo].pos, x_check = x_ref+y_ref-y_check, r = check_square(corner1, cells[y_ref*order_odd+x_check], cells[y_check*order_odd+x_check], cells[y_check*order_odd+x_ref]);
		add_score(total, r & 1, (r & 2) >> 1);
		backups_lo++;
	}
	if (total->changes_n > 0) {
		*backups_hi += 1;
	}
}

void check_row(int y_ref, int x_ref, int backups_lo, int *backups_hi, score_t *total) {
	int *corner1 = cells+y_ref*order_odd+x_ref;
	set_backup(backups+*backups_hi, x_ref, *corner1);
	set_score(total, 0, 0);
	while (backups_lo < *backups_hi) {
		int x_check = backups[backups_lo].pos, y_check = y_ref-x_ref+x_check, r = check_square(corner1, cells[y_check*order_odd+x_ref], cells[y_check*order_odd+x_check], cells[y_ref*order_odd+x_check]);
		add_score(total, r & 1, (r & 2) >> 1);
		backups_lo++;
	}
	if (total->changes_n > 0) {
		*backups_hi += 1;
	}
}

int check_square(int *corner1, int corner2, int corner3, int corner4) {
	if ((*corner1 & corner3) && corner2 == corner3 && corner3 == corner4) {
		if (*corner1 == corner3) {
			return 2;
		}
		*corner1 -= corner3;
		return 1;
	}
	return 0;
}

void set_backup(backup_t *backup, int pos, int cell) {
	backup->pos = pos;
	backup->cell = cell;
}

void set_score(score_t *score, int changes_n, int singles_n) {
	score->changes_n = changes_n;
	score->singles_n = singles_n;
}

void add_score(score_t *total, int changes_n, int singles_n) {
	total->changes_n += changes_n;
	total->singles_n += singles_n;
}

int compare_scores(score_t *score_a, score_t *score_b) {
	if (score_a->singles_n != score_b->singles_n) {
		return score_a->singles_n-score_b->singles_n;
	}
	return score_a->changes_n-score_b->changes_n;
}

void restore_row(int y, int backups_lo, int backups_hi) {
	while (backups_lo < backups_hi) {
		cells[y*order_odd+backups[backups_lo].pos] = backups[backups_lo].cell;
		backups_lo++;
	}
}

void restore_column(int x, int backups_lo, int backups_hi) {
	while (backups_lo < backups_hi) {
		cells[backups[backups_lo].pos*order_odd+x] = backups[backups_lo].cell;
		backups_lo++;
	}
}

int set_choices(int cell, int *choices) {
	int choices_n = 0, colors_idx, weight;
	for (colors_idx = 0, weight = 1; colors_idx < colors_n; colors_idx++, weight <<= 1) {
		if (cell & weight) {
			choices[choices_n++] = colors_idx;
		}
	}
	return choices_n;
}

int unique_choice(int cell) {
	int colors_idx, weight;
	for (colors_idx = 0, weight = 1; weight < cell; colors_idx++, weight <<= 1);
	if (weight == cell) {
		return colors_idx;
	}
	return colors_n;
}
