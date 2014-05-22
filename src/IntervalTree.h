/*
 * Copyright © 2011 Daniel Taliun, Christian Fuchsberger and Cristian Pattaro. All rights reserved.
 *
 * This file is part of GWAtoolbox.
 *
 * GWAtoolbox is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * GWAtoolbox is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with GWAtoolbox.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef INTERVALTREE_H_
#define INTERVALTREE_H_

#include <stdlib.h>
#include <map>
#include <iostream>
#include "Interval.h"

using namespace std;

template <class T>
class IntervalTree {
private:
	Interval<T>* root;

	/* Balancing */
	void balance(Interval<T>* interval);
	void rotate_left(Interval<T>* interval);
	void rotate_right(Interval<T>* interval);
	void replace_interval(Interval<T>* old_interval, Interval<T>* new_interval);
	void calculate_max_end(Interval<T>* interval);
	void remove_interval(Interval<T>* interval);
	void erase_interval(Interval<T>* interval);

	/* Queries */
	void get_intersecting_intervals(Interval<T>* interval, int position, vector<T>& result);
	void mark_intersecting_intervals(Interval<T>* interval, int position, int deviation);
	void get_intersecting_intervals(Interval<T>* interval, int start, int end, vector<T>& result);
	void mark_intersecting_intervals(Interval<T>* interval, int start, int end, int deviation);
	void get_intersecting_intervals(Interval<T>* interval, int start, int end, IntervalTree<T>& tree);
	void get_intersecting_intervals(Interval<T>* interval, int start, int end, int deviation, IntervalTree<T>& tree);
	int get_inordered(Interval<T>* interval, int k, vector<Interval<T>*>& result);
	int get_inordered_desc(Interval<T>* interval, int k, vector<Interval<T>*>& result);
	Interval<T>* get_nearest_from_right(Interval<T>* interval, int position, Interval<T>* nearest);
	Interval<T>* get_nearest_from_left(Interval<T>* interval, int position, Interval<T>* nearest);
	int get_knearest_from_right(Interval<T>* interval, int k, vector<Interval<T>*>& result);
	int get_knearest_from_left(Interval<T>* interval, int k, vector<Interval<T>*>& result);

	void get_marked_values(Interval<T>* interval, map<int, vector<T>* >& values);

	/* Properties */
	int get_values_count(Interval<T>* interval, int count);
	int get_intervals_count(Interval<T>* interval, int count);
	int get_height(Interval<T>* interval, int path_height, int max_height);

	/* Output */
	void print(ostream& stream, Interval<T>* interval, int intend);

public:
	IntervalTree();
	virtual ~IntervalTree();

	/* Insertion */
	void add(int start, int end, T& value);
	void add(int start, int end, T& value, int deviation);
	void add_all(int start, int end, vector<T>& values);
	void add_all(int start, int end, vector<T>& values, int deviation);

	void clear();

	/* Queries */
	void get_intersecting_intervals(int position, vector<T>& result);
	void mark_intersecting_intervals(int position, int deviation);
	void get_intersecting_intervals(int start, int end, vector<T>& result);
	void mark_intersecting_intervals(int start, int end, int deviation);
	void get_intersecting_intervals(int start, int end, IntervalTree<T>& tree);
	void get_intersecting_intervals(int start, int end, int deviation, IntervalTree<T>& tree);
	void get_inordered(int k, vector<Interval<T>*>& result);
	void get_inordered_desc(int k, vector<Interval<T>*>& result);
	Interval<T>* get_nearest_from_right(int position);
	Interval<T>* get_nearest_from_left(int position);
	void get_knearest_from_right(int position, int k, vector<Interval<T>*>& result);
	void get_knearest_from_left(int position, int k, vector<Interval<T>*>& result);

	void get_marked_values(map<int, vector<T>* >& values);

	/* Properties */
	int get_values_count();
	int get_intervals_count();
	int get_height();

	/* Output */
	void print(ostream& stream);
};

#endif
