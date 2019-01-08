#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define COLORS_MIN 2
#define BITS_N 8
#define ORDER_MIN 2
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
void evaluate_cell(int, int, direction_t, int, int, score_t *);
void choose_cell(int, int, int *, int, direction_t, int, int, int, int, int);
void check_column(int, int, int, int *, score_t *);
void check_row(int, int, int, int *, score_t *);
int check_square(int *, int, int, int);
void set_score(score_t *, int, int);
void add_score(score_t *, score_t *);
int compare_scores(score_t *, score_t *);
void set_backup(backup_t *, int, int);
void restore_row(int, int, int);
void restore_column(int, int, int);
void next_cell(int *, int *, direction_t *);
void print_cell(int);
int set_choices(int, int *);
int unique_choice(int);

int colors_n, order, singles_max, counter_period, order_odd, format_len, *cells,counter1, counter2, x_switch, y_switch, flag_switch;
unsigned time0;
backup_t *backups;

int main(void) {
	int colors_div, cells_size, x_first, y_first, cells_first, colors_all, colors_idx, cells_idx, *choices_all, symmetric;
	if (scanf("%d%d%d%d", &colors_n, &order, &singles_max, &counter_period) != 4 || colors_n < COLORS_MIN || (size_t)colors_n >= sizeof(int)*BITS_N || order < ORDER_MIN || singles_max < 0 || counter_period < 1) {
		fprintf(stderr, "Invalid parameters\n");
		fflush(stderr);
		return EXIT_FAILURE;
	}
	if (order%2 == 0) {
		order_odd = order+1;
	}
	else {
		order_odd = order;
	}
	for (colors_div = (colors_n-1)/10, format_len = 1; colors_div > 0; colors_div /= 10, format_len++);
	cells_size = order_odd*order_odd;
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
	choices_all = malloc(sizeof(int)*(size_t)(colors_n*cells_size));
	if (!choices_all) {
		fprintf(stderr, "Could not allocate memory for choices_all\n");
		fflush(stderr);
		free(cells);
		return EXIT_FAILURE;
	}
	backups = malloc(sizeof(backup_t)*(size_t)((colors_n-1)*cells_size));
	if (!backups) {
		fprintf(stderr, "Could not allocate memory for backups\n");
		fflush(stderr);
		free(choices_all);
		free(cells);
		return EXIT_FAILURE;
	}
	counter1 = 0;
	counter2 = 0;
	time0 = (unsigned)time(NULL);
	symmetric = ROT_CW_90+ROT_CW_180+ROT_CW_270+REF_AXIS_H+REF_AXIS_V+REF_DIAG_UB+REF_DIAG_BU;
	if (singles_max == 0) {
		erickson_matrix(x_first, y_first-1, choices_all, symmetric, WEST, 0, 2, 0);
	}
	else {
		direction_t direction_switch = NORTH;
		x_switch = x_first;
		y_switch = y_first;
		flag_switch = 1;
		printf("x_switch %d y_switch %d\n", x_switch, y_switch);
		fflush(stdout);
		erickson_matrix(x_first, y_first-1, choices_all, symmetric, WEST, 0, 2, 0);
		flag_switch = 0;
		next_cell(&x_switch, &y_switch, &direction_switch);
		while (y_switch >= 0 && y_switch < order) {
			printf("x_switch %d y_switch %d\n", x_switch, y_switch);
			fflush(stdout);
			erickson_matrix(x_first, y_first-1, choices_all, symmetric, WEST, 0, 2, 0);
			next_cell(&x_switch, &y_switch, &direction_switch);
		}
	}
	printf("Solutions %d*%d+%d Time %us\n", counter2, counter_period, counter1, (unsigned)time(NULL)-time0);
	fflush(stdout);
	free(backups);
	free(choices_all);
	free(cells);
	return EXIT_SUCCESS;
}

void erickson_matrix(int x, int y, int *choices, int symmetric, direction_t direction, int backups_lo, int colors_max, int singles_n) {
	int choices_n, choices_idx;
	if (y == -1 || y == order) {
		counter1++;
		if (counter1 == counter_period) {
			counter1 = 0;
			counter2++;
			printf("Solutions %d*%d Time %us\n", counter2, counter_period, (unsigned)time(NULL)-time0);
			fflush(stdout);
		}
		if (singles_n < singles_max || (counter1 == 1 && counter2 == 0)) {
			int y_cur;
			printf("Singles %d Time %us\n", singles_n, (unsigned)time(NULL)-time0);
			if (singles_n < singles_max) {
				singles_max = singles_n;
			}
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
		}
		return;
	}
	choices_n = set_choices(cells[y*order_odd+x], choices);
	if (singles_max == 0) {
		if (choices_n == 1) {
			if (choices[0] == colors_max-1 && colors_max < colors_n) {
				choose_cell(x, y, choices, symmetric, direction, backups_lo, colors_max+1, singles_n, cells[y*order_odd+x], 0);
			}
			else {
				choose_cell(x, y, choices, symmetric, direction, backups_lo, colors_max, singles_n, cells[y*order_odd+x], 0);
			}
		}
		else {
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
			for (choices_idx = 0; choices_idx < choices_n && choices[choices_idx] < colors_max; choices_idx++) {
				cells[y*order_odd+x] = 1 << choices[choices_idx];
				if (choices[choices_idx] == colors_max-1 && colors_max < colors_n) {
					choose_cell(x, y, choices, symmetric, direction, backups_lo, colors_max+1, singles_n, cells[y*order_odd+x], 1);
				}
				else {
					choose_cell(x, y, choices, symmetric, direction, backups_lo, colors_max, singles_n, cells[y*order_odd+x], 1);
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
	else {
		if (choices_n == 1) {
			if (x != x_switch || y != y_switch) {
				if (flag_switch) {
					symmetric = 0;
				}
				if (choices[0] == colors_max-1 && colors_max < colors_n) {
					choose_cell(x, y, choices, symmetric, direction, backups_lo, colors_max+1, singles_n, cells[y*order_odd+x], 0);
				}
				else {
					choose_cell(x, y, choices, symmetric, direction, backups_lo, colors_max, singles_n, cells[y*order_odd+x], 0);
				}
			}
		}
		else {
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
			if (x == x_switch && y == y_switch) {
				flag_switch = 1;
			}
			if (flag_switch) {
				int choice = choices[0];
				score_t score_min;
				cells[y*order_odd+x] = 1 << choices[0];
				evaluate_cell(x, y, direction, backups_lo, singles_n, &score_min);
				for (choices_idx = 1; choices_idx < choices_n && choices[choices_idx] < colors_max; choices_idx++) {
					score_t score;
					cells[y*order_odd+x] = 1 << choices[choices_idx];
					evaluate_cell(x, y, direction, backups_lo, singles_n, &score);
					if (compare_scores(&score, &score_min) <= 0) {
						choice = choices[choices_idx];
						score_min = score;
					}
				}
				if (score_min.singles_n <= singles_max) {
					if (x == x_switch && y == y_switch) {
						for (choices_idx = 0; choices_idx < choices_n && choices[choices_idx] < colors_max; choices_idx++) {
							if (choices[choices_idx] != choice) {
								cells[y*order_odd+x] = 1 << choices[choices_idx];
								if (choices[choices_idx] == colors_max-1 && colors_max < colors_n) {
									choose_cell(x, y, choices, 0, direction, backups_lo, colors_max+1, singles_n, cells[y*order_odd+x], 1);
								}
								else {
									choose_cell(x, y, choices, 0, direction, backups_lo, colors_max, singles_n, cells[y*order_odd+x], 1);
								}
							}
						}
					}
					else {
						cells[y*order_odd+x] = 1 << choice;
						if (choice == colors_max-1 && colors_max < colors_n) {
							choose_cell(x, y, choices, 0, direction, backups_lo, colors_max+1, singles_n, cells[y*order_odd+x], 1);
						}
						else {
							choose_cell(x, y, choices, 0, direction, backups_lo, colors_max, singles_n, cells[y*order_odd+x], 1);
						}
					}
				}
			}
			else {
				for (choices_idx = 0; choices_idx < choices_n && choices[choices_idx] < colors_max; choices_idx++) {
					cells[y*order_odd+x] = 1 << choices[choices_idx];
					if (choices[choices_idx] == colors_max-1 && colors_max < colors_n) {
						choose_cell(x, y, choices, symmetric, direction, backups_lo, colors_max+1, singles_n, cells[y*order_odd+x], 1);
					}
					else {
						choose_cell(x, y, choices, symmetric, direction, backups_lo, colors_max, singles_n, cells[y*order_odd+x], 1);
					}
				}
			}
			if (x == x_switch && y == y_switch) {
				flag_switch = 0;
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
}

void evaluate_cell(int x, int y, direction_t direction, int backups_lo, int singles_n, score_t *total) {
	switch (direction) {
		int backups_hi, y_min, y_ref, x_ref;
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
			for (y_ref = y-1; y_ref >= y_min && total->singles_n <= singles_max; y_ref--) {
				check_column(x, y_ref, backups_lo, &backups_hi, &score);
				add_score(total, &score);
			}
			if (total->singles_n > singles_max) {
				set_score(total, 0, singles_max+1);
			}
			restore_column(x, backups_lo+1, backups_hi);
			break;
		case WEST:
			backups_hi = backups_lo+1;
			set_score(total, 0, singles_n);
			for (x_ref = x-1; x_ref >= y && total->singles_n <= singles_max; x_ref--) {
				check_row(y, x_ref, backups_lo, &backups_hi, &score);
				add_score(total, &score);
			}
			if (total->singles_n > singles_max) {
				set_score(total, 0, singles_max+1);
			}
			restore_row(y, backups_lo+1, backups_hi);
			break;
		case SOUTH:
			backups_hi = backups_lo+1;
			set_score(total, 0, singles_n);
			for (y_ref = y+1; y_ref <= order_odd-1-x && total->singles_n <= singles_max; y_ref++) {
				check_column(x, y_ref, backups_lo, &backups_hi, &score);
				add_score(total, &score);
			}
			if (total->singles_n > singles_max) {
				set_score(total, 0, singles_max+1);
			}
			restore_column(x, backups_lo+1, backups_hi);
			break;
		case EAST:
			backups_hi = backups_lo+1;
			set_score(total, 0, singles_n);
			for (x_ref = x+1; x_ref <= y && total->singles_n <= singles_max; x_ref++) {
				check_row(y, x_ref, backups_lo, &backups_hi, &score);
				add_score(total, &score);
			}
			if (total->singles_n > singles_max) {
				set_score(total, 0, singles_max+1);
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
		int x_opp = order_odd-1-x, y_opp = order_odd-1-y, cell_rot_cw_90 = cells[x*order_odd+y_opp], cell_rot_cw_180 = cells[y_opp*order_odd+x_opp], cell_rot_cw_270 = cells[x_opp*order_odd+y], cell_ref_axis_h = cells[y_opp*order_odd+x], cell_ref_axis_v = cells[y*order_odd+x_opp], cell_ref_diag_ub = cells[x*order_odd+y], cell_ref_diag_bu = cells[x_opp*order_odd+y_opp];
		if ((symmetric & ROT_CW_90) && unique_choice(cell_rot_cw_90) == 1) {
			if (choice < cell_rot_cw_90) {
				return;
			}
			if (choice > cell_rot_cw_90) {
				symmetric -= ROT_CW_90;
			}
		}
		if ((symmetric & ROT_CW_180) && unique_choice(cell_rot_cw_180) == 1) {
			if (choice < cell_rot_cw_180) {
				return;
			}
			if (choice > cell_rot_cw_180) {
				symmetric -= ROT_CW_180;
			}
		}
		if ((symmetric & ROT_CW_270) && unique_choice(cell_rot_cw_270) == 1) {
			if (choice < cell_rot_cw_270) {
				return;
			}
			if (choice > cell_rot_cw_270) {
				symmetric -= ROT_CW_270;
			}
		}
		if ((symmetric & REF_AXIS_H) && unique_choice(cell_ref_axis_h) == 1) {
			if (choice < cell_ref_axis_h) {
				return;
			}
			if (choice > cell_ref_axis_h) {
				symmetric -= REF_AXIS_H;
			}
		}
		if ((symmetric & REF_AXIS_V) && unique_choice(cell_ref_axis_v) == 1) {
			if (choice < cell_ref_axis_v) {
				return;
			}
			if (choice > cell_ref_axis_v) {
				symmetric -= REF_AXIS_V;
			}
		}
		if ((symmetric & REF_DIAG_UB) && unique_choice(cell_ref_diag_ub) == 1) {
			if (choice < cell_ref_diag_ub) {
				return;
			}
			if (choice > cell_ref_diag_ub) {
				symmetric -= REF_DIAG_UB;
			}
		}
		if ((symmetric & REF_DIAG_BU) && unique_choice(cell_ref_diag_bu) == 1) {
			if (choice < cell_ref_diag_bu) {
				return;
			}
			if (choice > cell_ref_diag_bu) {
				symmetric -= REF_DIAG_BU;
			}
		}
	}
	direction_bak = direction;
	switch (direction_bak) {
		case NORTH:
			if (lookahead) {
				int backups_hi = backups_lo+1, y_ref;
				score_t score;
				for (y_ref = y-1; y_ref >= order_odd-1-x && singles_n <= singles_max; y_ref--) {
					check_column(x, y_ref, backups_lo, &backups_hi, &score);
					singles_n += score.singles_n;
				}
				if (singles_n <= singles_max) {
					next_cell(&x, &y, &direction);
					erickson_matrix(x, y, choices+colors_n, symmetric, direction, backups_hi, colors_max, singles_n);
				}
				restore_column(x, backups_lo+1, backups_hi);
			}
			else {
				next_cell(&x, &y, &direction);
				erickson_matrix(x, y, choices+colors_n, symmetric, direction, backups_lo, colors_max, singles_n);
			}
			break;
		case WEST:
			if (lookahead) {
				int backups_hi = backups_lo+1, x_ref;
				score_t score;
				for (x_ref = x-1; x_ref >= y && singles_n <= singles_max; x_ref--) {
					check_row(y, x_ref, backups_lo, &backups_hi, &score);
					singles_n += score.singles_n;
				}
				if (singles_n <= singles_max) {
					next_cell(&x, &y, &direction);
					erickson_matrix(x, y, choices+colors_n, symmetric, direction, backups_hi, colors_max, singles_n);
				}
				restore_row(y, backups_lo+1, backups_hi);
			}
			else {
				next_cell(&x, &y, &direction);
				erickson_matrix(x, y, choices+colors_n, symmetric, direction, backups_lo, colors_max, singles_n);
			}
			break;
		case SOUTH:
			if (lookahead) {
				int backups_hi = backups_lo+1, y_ref;
				score_t score;
				for (y_ref = y+1; y_ref <= order_odd-1-x && singles_n <= singles_max; y_ref++) {
					check_column(x, y_ref, backups_lo, &backups_hi, &score);
					singles_n += score.singles_n;
				}
				if (singles_n <= singles_max) {
					next_cell(&x, &y, &direction);
					erickson_matrix(x, y, choices+colors_n, symmetric, direction, backups_hi, colors_max, singles_n);
				}
				restore_column(x, backups_lo+1, backups_hi);
			}
			else {
				next_cell(&x, &y, &direction);
				erickson_matrix(x, y, choices+colors_n, symmetric, direction, backups_lo, colors_max, singles_n);
			}
			break;
		case EAST:
			if (lookahead) {
				int backups_hi = backups_lo+1, x_ref;
				score_t score;
				for (x_ref = x+1; x_ref <= y && singles_n <= singles_max; x_ref++) {
					check_row(y, x_ref, backups_lo, &backups_hi, &score);
					singles_n += score.singles_n;
				}
				if (singles_n <= singles_max) {
					next_cell(&x, &y, &direction);
					erickson_matrix(x, y, choices+colors_n, symmetric, direction, backups_hi, colors_max, singles_n);
				}
				restore_row(y, backups_lo+1, backups_hi);
			}
			else {
				next_cell(&x, &y, &direction);
				erickson_matrix(x, y, choices+colors_n, symmetric, direction, backups_lo, colors_max, singles_n);
			}
			break;
		default:
			fprintf(stderr, "This should never happen\n");
			fflush(stderr);
	}
}

void check_column(int x_ref, int y_ref, int backups_lo, int *backups_hi, score_t *total) {
	int *corner1 = cells+y_ref*order_odd+x_ref;
	set_backup(backups+*backups_hi, y_ref, *corner1);
	set_score(total, 0, 0);
	for (; backups_lo < *backups_hi; backups_lo++) {
		int y_check = backups[backups_lo].pos, x_check = x_ref+y_ref-y_check, r = check_square(corner1, cells[y_ref*order_odd+x_check], cells[y_check*order_odd+x_check], cells[y_check*order_odd+x_ref]);
		score_t score;
		set_score(&score, r & 1, (r & 2) >> 1);
		add_score(total, &score);
	}
	if (total->changes_n > 0) {
		*backups_hi += 1;
	}
}

void check_row(int y_ref, int x_ref, int backups_lo, int *backups_hi, score_t *total) {
	int *corner1 = cells+y_ref*order_odd+x_ref;
	set_backup(backups+*backups_hi, x_ref, *corner1);
	set_score(total, 0, 0);
	for (; backups_lo < *backups_hi; backups_lo++) {
		int x_check = backups[backups_lo].pos, y_check = y_ref-x_ref+x_check, r = check_square(corner1, cells[y_check*order_odd+x_ref], cells[y_check*order_odd+x_check], cells[y_ref*order_odd+x_check]);
		score_t score;
		set_score(&score, r & 1, (r & 2) >> 1);
		add_score(total, &score);
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

void set_backup(backup_t *backup, int pos, int cell) {
	backup->pos = pos;
	backup->cell = cell;
}

void restore_row(int y, int backups_lo, int backups_hi) {
	for (; backups_lo < backups_hi; backups_lo++) {
		cells[y*order_odd+backups[backups_lo].pos] = backups[backups_lo].cell;
	}
}

void restore_column(int x, int backups_lo, int backups_hi) {
	for (; backups_lo < backups_hi; backups_lo++) {
		cells[backups[backups_lo].pos*order_odd+x] = backups[backups_lo].cell;
	}
}

void next_cell(int *x, int *y, direction_t *direction) {
	direction_t direction_bak = *direction;
	switch (direction_bak) {
		case NORTH:
			*y -= 1;
			if (order_odd-2-*x == *y) {
				*direction = WEST;
			}
			break;
		case WEST:
			*x -= 1;
			if (*x == *y) {
				*direction = SOUTH;
			}
			break;
		case SOUTH:
			*y += 1;
			if (*x == order_odd-1-*y) {
				*direction = EAST;
			}
			break;
		case EAST:
			*x += 1;
			if (*x == *y) {
				*direction = NORTH;
			}
			break;
		default:
			fprintf(stderr, "This should never happen\n");
			fflush(stderr);
	}
}

void print_cell(int cell) {
	if (unique_choice(cell) == 1) {
		int choice;
		set_choices(cell, &choice);
		printf("%0*d", format_len, choice);
	}
	else {
		fprintf(stderr, "This should never happen\n");
		fflush(stderr);
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
	int choices_n = 0, colors_idx, weight;
	for (colors_idx = 0, weight = 1; colors_idx < colors_n && choices_n < 2; colors_idx++, weight <<= 1) {
		if (cell & weight) {
			choices_n++;
		}
	}
	return choices_n == 1;
}
