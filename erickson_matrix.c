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
	int choices_n;
	int mask;
}
cell_t;

typedef struct {
	int pos;
	cell_t data;
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

void erickson_matrix(int, int, int, direction_t, int, int);
void evaluate_cell(int, int, direction_t, int, int, score_t *);
void choose_cell(int, int, int, direction_t, int, int, int, int);
void check_column(int, int, int, int *, score_t *);
void check_row(int, int, int, int *, score_t *);
int check_square(cell_t *, int, int, int);
void set_score(score_t *, int, int);
void add_score(score_t *, score_t *);
int compare_scores(score_t *, score_t *);
void set_backup(backup_t *, int, cell_t *);
void restore_row(int, int, int);
void restore_column(int, int, int);
void set_cell(cell_t *, int, int);
void next_cell(int *, int *, direction_t *);
void print_cell(cell_t *);

int colors_n, order, singles_max, counter_period, order_odd, format_len, counter1, counter2, x_switch, y_switch, flag_switch;
unsigned time0;
cell_t *cells;
backup_t *backups;

int main(void) {
	int colors_div, cells_size, x_first, y_first, cells_first, colors_all, colors_idx, cells_idx, symmetric;
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
	cells = malloc(sizeof(cell_t)*(size_t)cells_size);
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
		set_cell(cells+cells_idx, colors_n, colors_all);
	}
	set_cell(cells+cells_idx, 1, 1);
	for (cells_idx++; cells_idx < cells_size; cells_idx++) {
		set_cell(cells+cells_idx, colors_n, colors_all);
	}
	backups = malloc(sizeof(backup_t)*(size_t)((colors_n-1)*cells_size));
	if (!backups) {
		fprintf(stderr, "Could not allocate memory for backups\n");
		fflush(stderr);
		free(cells);
		return EXIT_FAILURE;
	}
	counter1 = 0;
	counter2 = 0;
	time0 = (unsigned)time(NULL);
	symmetric = ROT_CW_90+ROT_CW_180+ROT_CW_270+REF_AXIS_H+REF_AXIS_V+REF_DIAG_UB+REF_DIAG_BU;
	if (singles_max == 0) {
		erickson_matrix(x_first, y_first-1, symmetric, WEST, 0, 0);
	}
	else {
		direction_t direction_switch = NORTH;
		x_switch = x_first;
		y_switch = y_first;
		flag_switch = 1;
		printf("x_switch %d y_switch %d\n", x_switch, y_switch);
		fflush(stdout);
		erickson_matrix(x_first, y_first-1, symmetric, WEST, 0, 0);
		flag_switch = 0;
		next_cell(&x_switch, &y_switch, &direction_switch);
		while (y_switch >= 0 && y_switch < order) {
			printf("x_switch %d y_switch %d\n", x_switch, y_switch);
			fflush(stdout);
			erickson_matrix(x_first, y_first-1, symmetric, WEST, 0, 0);
			next_cell(&x_switch, &y_switch, &direction_switch);
		}
	}
	printf("Solutions %d*%d+%d Time %us\n", counter2, counter_period, counter1, (unsigned)time(NULL)-time0);
	fflush(stdout);
	free(backups);
	free(cells);
	return EXIT_SUCCESS;
}

void erickson_matrix(int x, int y, int symmetric, direction_t direction, int backups_lo, int singles_n) {
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
				print_cell(cells+y_cur*order_odd);
				for (x_cur = 1; x_cur < order; x_cur++) {
					putchar(' ');
					print_cell(cells+y_cur*order_odd+x_cur);
				}
				puts("");
			}
			fflush(stdout);
		}
		return;
	}
	if (singles_max == 0) {
		if (cells[y*order_odd+x].choices_n == 1) {
			choose_cell(x, y, symmetric, direction, backups_lo, singles_n, cells[y*order_odd+x].mask, 0);
		}
		else {
			int colors_idx, weight;
			switch (direction) {
				case NORTH:
				case SOUTH:
					set_backup(backups+backups_lo, y, cells+y*order_odd+x);
					break;
				case WEST:
				case EAST:
					set_backup(backups+backups_lo, x, cells+y*order_odd+x);
					break;
				default:
					fprintf(stderr, "This should never happen\n");
					fflush(stderr);
					return;
			}
			for (colors_idx = 0, weight = 1; colors_idx < colors_n; colors_idx++, weight <<= 1) {
				if (backups[backups_lo].data.mask & weight) {
					set_cell(cells+y*order_odd+x, 1, weight);
					choose_cell(x, y, symmetric, direction, backups_lo, singles_n, weight, 1);
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
		if (cells[y*order_odd+x].choices_n == 1) {
			if (x != x_switch || y != y_switch) {
				if (flag_switch) {
					symmetric = 0;
				}
				choose_cell(x, y, symmetric, direction, backups_lo, singles_n, cells[y*order_odd+x].mask, 0);
			}
		}
		else {
			int colors_idx, weight;
			switch (direction) {
				case NORTH:
				case SOUTH:
					set_backup(backups+backups_lo, y, cells+y*order_odd+x);
					break;
				case WEST:
				case EAST:
					set_backup(backups+backups_lo, x, cells+y*order_odd+x);
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
				int choice = 0;
				score_t score_min;
				set_score(&score_min, 0, singles_max+1);
				for (colors_idx = 0, weight = 1; colors_idx < colors_n; colors_idx++, weight <<= 1) {
					score_t score;
					if (backups[backups_lo].data.mask & weight) {
						set_cell(cells+y*order_odd+x, 1, weight);
						evaluate_cell(x, y, direction, backups_lo, singles_n, &score);
						if (compare_scores(&score, &score_min) <= 0) {
							choice = weight;
							score_min = score;
						}
					}
				}
				if (score_min.singles_n <= singles_max) {
					if (x == x_switch && y == y_switch) {
						for (colors_idx = 0, weight = 1; colors_idx < colors_n; colors_idx++, weight <<= 1) {
							if ((backups[backups_lo].data.mask & weight) && weight != choice) {
								set_cell(cells+y*order_odd+x, 1, weight);
								choose_cell(x, y, 0, direction, backups_lo, singles_n, weight, 1);
							}
						}
					}
					else {
						set_cell(cells+y*order_odd+x, 1, choice);
						choose_cell(x, y, 0, direction, backups_lo, singles_n, choice, 1);
					}
				}
			}
			else {
				for (colors_idx = 0, weight = 1; colors_idx < colors_n; colors_idx++, weight <<= 1) {
					if (backups[backups_lo].data.mask & weight) {
						set_cell(cells+y*order_odd+x, 1, weight);
						choose_cell(x, y, symmetric, direction, backups_lo, singles_n, weight, 1);
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

void choose_cell(int x, int y, int symmetric, direction_t direction, int backups_lo, int singles_n, int choice, int lookahead) {
	direction_t direction_bak;
	if (symmetric) {
		int x_opp = order_odd-1-x, y_opp = order_odd-1-y;
		cell_t *cell_rot_cw_90 = cells+x*order_odd+y_opp, *cell_rot_cw_180 = cells+y_opp*order_odd+x_opp, *cell_rot_cw_270 = cells+x_opp*order_odd+y, *cell_ref_axis_h = cells+y_opp*order_odd+x, *cell_ref_axis_v = cells+y*order_odd+x_opp, *cell_ref_diag_ub = cells+x*order_odd+y, *cell_ref_diag_bu = cells+x_opp*order_odd+y_opp;
		if ((symmetric & ROT_CW_90) && cell_rot_cw_90->choices_n == 1) {
			if (choice < cell_rot_cw_90->mask) {
				return;
			}
			if (choice > cell_rot_cw_90->mask) {
				symmetric -= ROT_CW_90;
			}
		}
		if ((symmetric & ROT_CW_180) && cell_rot_cw_180->choices_n == 1) {
			if (choice < cell_rot_cw_180->mask) {
				return;
			}
			if (choice > cell_rot_cw_180->mask) {
				symmetric -= ROT_CW_180;
			}
		}
		if ((symmetric & ROT_CW_270) && cell_rot_cw_270->choices_n == 1) {
			if (choice < cell_rot_cw_270->mask) {
				return;
			}
			if (choice > cell_rot_cw_270->mask) {
				symmetric -= ROT_CW_270;
			}
		}
		if ((symmetric & REF_AXIS_H) && cell_ref_axis_h->choices_n == 1) {
			if (choice < cell_ref_axis_h->mask) {
				return;
			}
			if (choice > cell_ref_axis_h->mask) {
				symmetric -= REF_AXIS_H;
			}
		}
		if ((symmetric & REF_AXIS_V) && cell_ref_axis_v->choices_n == 1) {
			if (choice < cell_ref_axis_v->mask) {
				return;
			}
			if (choice > cell_ref_axis_v->mask) {
				symmetric -= REF_AXIS_V;
			}
		}
		if ((symmetric & REF_DIAG_UB) && cell_ref_diag_ub->choices_n == 1) {
			if (choice < cell_ref_diag_ub->mask) {
				return;
			}
			if (choice > cell_ref_diag_ub->mask) {
				symmetric -= REF_DIAG_UB;
			}
		}
		if ((symmetric & REF_DIAG_BU) && cell_ref_diag_bu->choices_n == 1) {
			if (choice < cell_ref_diag_bu->mask) {
				return;
			}
			if (choice > cell_ref_diag_bu->mask) {
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
					erickson_matrix(x, y, symmetric, direction, backups_hi, singles_n);
				}
				restore_column(x, backups_lo+1, backups_hi);
			}
			else {
				next_cell(&x, &y, &direction);
				erickson_matrix(x, y, symmetric, direction, backups_lo, singles_n);
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
					erickson_matrix(x, y, symmetric, direction, backups_hi, singles_n);
				}
				restore_row(y, backups_lo+1, backups_hi);
			}
			else {
				next_cell(&x, &y, &direction);
				erickson_matrix(x, y, symmetric, direction, backups_lo, singles_n);
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
					erickson_matrix(x, y, symmetric, direction, backups_hi, singles_n);
				}
				restore_column(x, backups_lo+1, backups_hi);
			}
			else {
				next_cell(&x, &y, &direction);
				erickson_matrix(x, y, symmetric, direction, backups_lo, singles_n);
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
					erickson_matrix(x, y, symmetric, direction, backups_hi, singles_n);
				}
				restore_row(y, backups_lo+1, backups_hi);
			}
			else {
				next_cell(&x, &y, &direction);
				erickson_matrix(x, y, symmetric, direction, backups_lo, singles_n);
			}
			break;
		default:
			fprintf(stderr, "This should never happen\n");
			fflush(stderr);
	}
}

void check_column(int x_ref, int y_ref, int backups_lo, int *backups_hi, score_t *total) {
	cell_t *corner1 = cells+y_ref*order_odd+x_ref;
	set_backup(backups+*backups_hi, y_ref, corner1);
	set_score(total, 0, 0);
	for (; backups_lo < *backups_hi; backups_lo++) {
		int y_check = backups[backups_lo].pos, x_check = x_ref+y_ref-y_check, r = check_square(corner1, cells[y_ref*order_odd+x_check].mask, cells[y_check*order_odd+x_check].mask, cells[y_check*order_odd+x_ref].mask);
		score_t score;
		set_score(&score, r & 1, (r & 2) >> 1);
		add_score(total, &score);
	}
	if (total->changes_n > 0) {
		*backups_hi += 1;
	}
}

void check_row(int y_ref, int x_ref, int backups_lo, int *backups_hi, score_t *total) {
	cell_t *corner1 = cells+y_ref*order_odd+x_ref;
	set_backup(backups+*backups_hi, x_ref, corner1);
	set_score(total, 0, 0);
	for (; backups_lo < *backups_hi; backups_lo++) {
		int x_check = backups[backups_lo].pos, y_check = y_ref-x_ref+x_check, r = check_square(corner1, cells[y_check*order_odd+x_ref].mask, cells[y_check*order_odd+x_check].mask, cells[y_ref*order_odd+x_check].mask);
		score_t score;
		set_score(&score, r & 1, (r & 2) >> 1);
		add_score(total, &score);
	}
	if (total->changes_n > 0) {
		*backups_hi += 1;
	}
}

int check_square(cell_t *corner1, int corner2, int corner3, int corner4) {
	if ((corner1->mask & corner3) && corner2 == corner3 && corner3 == corner4) {
		if (corner1->mask == corner3) {
			return 2;
		}
		set_cell(corner1, corner1->choices_n-1, corner1->mask-corner3);
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

void set_backup(backup_t *backup, int pos, cell_t *cell) {
	backup->pos = pos;
	backup->data = *cell;
}

void restore_row(int y, int backups_lo, int backups_hi) {
	for (; backups_lo < backups_hi; backups_lo++) {
		cells[y*order_odd+backups[backups_lo].pos] = backups[backups_lo].data;
	}
}

void restore_column(int x, int backups_lo, int backups_hi) {
	for (; backups_lo < backups_hi; backups_lo++) {
		cells[backups[backups_lo].pos*order_odd+x] = backups[backups_lo].data;
	}
}

void set_cell(cell_t *cell, int choices_n, int mask) {
	cell->choices_n = choices_n;
	cell->mask = mask;
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

void print_cell(cell_t *cell) {
	if (cell->choices_n == 1) {
		int colors_idx, weight;
		for (colors_idx = 0, weight = 1; colors_idx < colors_n && (cell->mask & weight) == 0; colors_idx++, weight <<= 1);
		if (colors_idx < colors_n) {
			printf("%0*d", format_len, colors_idx);
		}
		else {
			fprintf(stderr, "This should never happen\n");
			fflush(stderr);
		}
	}
	else {
		fprintf(stderr, "This should never happen\n");
		fflush(stderr);
	}
}
