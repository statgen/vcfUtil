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

#include "IntervalTree.h"

template <class T>
IntervalTree<T>::IntervalTree():root(NULL) {

}

template <class T>
IntervalTree<T>::~IntervalTree() {
	erase_interval(root);
	root = NULL;
}

template <class T>
void IntervalTree<T>::add(int start, int end, T& value) {
	add(start, end, value, 0);
}

template <class T>
void IntervalTree<T>::add(int start, int end, T& value, int deviation) {
	Interval<T>* new_interval = new Interval<T>(start, end, end, value, deviation, Interval<T>::RED, NULL, NULL);

	if (root == NULL) {
		root = new_interval;
	}
	else {
		Interval<T>* parent = root;
		int comparison = 0;

		while(true) {
			comparison = new_interval->compare(*parent);

			if (comparison == 0) {
				(parent->values).push_back(value);
				delete new_interval;
				return;
			}
			else if (comparison < 0) {
				parent->max = max(parent->max, new_interval->max);

				if (parent->left == NULL) {
					parent->left = new_interval;
					break;
				}
				else {
					parent = parent->left;
				}
			}
			else {
				parent->max = max(parent->max, new_interval->max);

				if (parent->right == NULL) {
					parent->right = new_interval;
					break;
				}
				else {
					parent = parent->right;
				}
			}
		}

		new_interval->parent = parent;
	}

	balance(new_interval);
}

template <class T>
void IntervalTree<T>::add_all(int start, int end, vector<T>& values) {
	add_all(start, end, values, 0);
}

template <class T>
void IntervalTree<T>::add_all(int start, int end, vector<T>& values, int deviation) {
	Interval<T>* new_interval = new Interval<T>(start, end, end, values, deviation, Interval<T>::RED, NULL, NULL);

	if (root == NULL) {
		root = new_interval;
	}
	else {
		Interval<T>* parent = root;
		int comparison = 0;

		while(true) {
			comparison = new_interval->compare(*parent);

			if (comparison == 0) {
				parent->values.insert(parent->values.end(), values.begin(), values.end());
				delete new_interval;
				return;
			}
			else if (comparison < 0) {
				parent->max = max(parent->max, new_interval->max);

				if (parent->left == NULL) {
					parent->left = new_interval;
					break;
				}
				else {
					parent = parent->left;
				}
			}
			else {
				parent->max = max(parent->max, new_interval->max);

				if (parent->right == NULL) {
					parent->right = new_interval;
					break;
				}
				else {
					parent = parent->right;
				}
			}
		}

		new_interval->parent = parent;
	}

	balance(new_interval);
}

/* Ensures correctness and balansing of a tree. */
template <class T>
void IntervalTree<T>::balance(Interval<T>* interval) {
	/* Ensures that root node is always black */
	if (interval->parent == NULL) {
		interval->color = Interval<T>::BLACK;
	}
	/* Ensures that the parent node is black */
	else if (interval->parent->color != Interval<T>::BLACK) {
		Interval<T>* uncle = interval->uncle();
		Interval<T>* grandparent = interval->grandparent();

		/* Ensures that all children of red nodes are black */
		if ((uncle != NULL) && (uncle->color == Interval<T>::RED)) {
			interval->parent->color = Interval<T>::BLACK;
			uncle->color = Interval<T>::BLACK;
			grandparent->color = Interval<T>::RED;

			balance(grandparent);
		}
		else {
			/* Unify cases for the last step with appropriate rotations */
			if ((interval == interval->parent->right) && (interval->parent == grandparent->left)) {
				rotate_left(interval->parent);
				interval = interval->left;
			}
			else if ((interval == interval->parent->left) && (interval->parent == grandparent->right)) {
				rotate_right(interval->parent);
				interval = interval->right;
			}

			/* The last step. Adjust colors and rotate. */
			grandparent = interval->grandparent(); /* Grandparen is the same. We do not need this assignment. */

			interval->parent->color = Interval<T>::BLACK;
			grandparent->color = Interval<T>::RED;

			if ((interval == interval->parent->left) && (interval->parent == grandparent->left)) {
				rotate_right(grandparent);
			}
			else {
				rotate_left(grandparent);
			}
		}
	}
}

/* Rotates tree to the left with respect to provided interval. */
template <class T>
void IntervalTree<T>::rotate_left(Interval<T>* interval) {
	Interval<T>* right_interval = interval->right;

	replace_interval(interval, right_interval);

	interval->right = right_interval->left;
	if (right_interval->left != NULL) {
		right_interval->left->parent = interval;
	}
	right_interval->left = interval;
	interval->parent = right_interval;

	calculate_max_end(interval);
	calculate_max_end(right_interval);
}

/* Rotates tree to the right with respect to provided interval. */
template <class T>
void IntervalTree<T>::rotate_right(Interval<T>* interval) {
	Interval<T>* left_interval = interval->left;

	replace_interval(interval, left_interval);

	interval->left = left_interval->right;
	if (left_interval->right != NULL) {
		left_interval->right->parent = interval;
	}
	left_interval->right = interval;
	interval->parent = left_interval;

	calculate_max_end(interval);
	calculate_max_end(left_interval);
}

/*
 *	Replaces one interval with another.
 *	Note:	Maximum end points are not recalculated. This method is used only in tree rotations.
 *			It shouldn't be used as stand-alone, since doesn't reflect changes to upper levels.
 */
template <class T>
void IntervalTree<T>::replace_interval(Interval<T>* old_interval, Interval<T>* new_interval) {
	if (old_interval->parent == NULL) {
		root = new_interval;
	}
	else {
		if (old_interval == old_interval->parent->left) {
			old_interval->parent->left = new_interval;
		}
		else {
			old_interval->parent->right = new_interval;
		}
	}

	if (new_interval != NULL) {
		new_interval->parent = old_interval->parent;
	}
}

/*
 *	Calculates the maximum end point of provided interval node and its children.
 *	Node:	It assumes, that both children already contain correct max value.
 */
template <class T>
void IntervalTree<T>::calculate_max_end(Interval<T>* interval) {
	if (interval->left == NULL) {
		if (interval->right == NULL) {
			interval->max = interval->end;
		}
		else {
			interval->max = max(interval->end, interval->right->max);
		}
	}
	else {
		if (interval->right == NULL) {
			interval->max = max(interval->end, interval->left->max);
		}
		else {
			interval->max = max(interval->end, max(interval->left->max, interval->right->max));
		}
	}
}

/* Returns values of all intervals that intersect with provided position. */
template <class T>
void IntervalTree<T>::get_intersecting_intervals(int position, vector<T>& result) {
	get_intersecting_intervals(root, position, result);
}

/* Returns values of all intervals that intersect with provided position. */
template <class T>
void IntervalTree<T>::get_intersecting_intervals(Interval<T>* interval, int position, vector<T>& result) {
	if (interval != NULL) {
		if ((interval->start <= position) && (position <= interval->end)) {
			result.insert(result.end(), interval->values.begin(), interval->values.end());
		}

		if ((interval->left != NULL) && (interval->left->max >= position)) {
			get_intersecting_intervals(interval->left, position, result);
		}

		if ((position >= interval->start) && (interval->right != NULL) && (interval->right->max >= position)) {
			get_intersecting_intervals(interval->right, position, result);
		}
	}
}

/* Marks all intervals in a tree that intersect with provided position. */
template <class T>
void IntervalTree<T>::mark_intersecting_intervals(int position, int deviation) {
	mark_intersecting_intervals(root, position, deviation);
}


/* Marks all intervals in a tree that intersect with provided position. */
template <class T>
void IntervalTree<T>::mark_intersecting_intervals(Interval<T>* interval, int position, int deviation) {
	if (interval != NULL) {
		if ((interval->start <= position) && (position <= interval->end)) {
			interval->deviation = deviation;
		}

		if ((interval->left != NULL) && (interval->left->max >= position)) {
			mark_intersecting_intervals(interval->left, position, deviation);
		}

		if ((position >= interval->start) && (interval->right != NULL) && (interval->right->max >= position)) {
			mark_intersecting_intervals(interval->right, position, deviation);
		}
	}

}

/*
 *	Returns values of all intervals that intersect with provided interval.
 *	Note:	It is assumed that provided start point is equal to or greater than provided end point.
 */
template <class T>
void IntervalTree<T>::get_intersecting_intervals(int start, int end, vector<T>& result) {
	get_intersecting_intervals(root, start, end, result);
}

/*
 *	Returns values of all intervals that intersect with provided interval.
 *	Note:	It is assumed that provided start point is equal to or greater than provided end point.
 */
template <class T>
void IntervalTree<T>::get_intersecting_intervals(Interval<T>* interval, int start, int end, vector<T>& result) {
	if (interval != NULL) {
		if ((start <= interval->end) && (end >= interval->start)) {
			result.insert(result.end(), interval->values.begin(), interval->values.end());
		}

		if ((interval->left != NULL) && (interval->left->max >= start)) {
			get_intersecting_intervals(interval->left, start, end, result);
		}

		if ((end >= interval->start) && (interval->right != NULL) && (interval->right->max >= start)) {
			get_intersecting_intervals(interval->right, start, end, result);
		}
	}
}

/*
 *	Marks all intervals in a tree that intersect with provided interval.
 *	Note:	It is assumed that provided start point is equal to or greater than provided end point.
 */
template <class T>
void IntervalTree<T>::mark_intersecting_intervals(int start, int end, int deviation) {
	mark_intersecting_intervals(root, start, end, deviation);
}

/*
 *	Marks all intervals in a tree that intersect with provided interval.
 *	Note:	It is assumed that provided start point is equal to or greater than provided end point.
 */
template <class T>
void IntervalTree<T>::mark_intersecting_intervals(Interval<T>* interval, int start, int end, int deviation) {
	if (interval != NULL) {
		if ((start <= interval->end) && (end >= interval->start)) {
			interval->deviation = deviation;
		}

		if ((interval->left != NULL) && (interval->left->max >= start)) {
			mark_intersecting_intervals(interval->left, start, end, deviation);
		}

		if ((end >= interval->start) && (interval->right != NULL) && (interval->right->max >= start)) {
			mark_intersecting_intervals(interval->right, start, end, deviation);
		}
	}
}

/*
 *	Returns values of all intervals that intersect with provided interval.
 *	Note:	It is assumed that provided start point is equal to or greater than provided end point.
 */
template <class T>
void IntervalTree<T>::get_intersecting_intervals(int start, int end, IntervalTree<T>& tree) {
	get_intersecting_intervals(root, start, end, tree);
}

/*
 *	Returns values of all intervals that intersect with provided interval.
 *	Note:	It is assumed that provided start point is equal to or greater than provided end point.
 */
template <class T>
void IntervalTree<T>::get_intersecting_intervals(Interval<T>* interval, int start, int end, IntervalTree<T>& tree) {
	if (interval != NULL) {
		if ((start <= interval->end) && (end >= interval->start)) {
			tree.add_all(interval->start, interval->end, interval->values);
		}

		if ((interval->left != NULL) && (interval->left->max >= start)) {
			get_intersecting_intervals(interval->left, start, end, tree);
		}

		if ((end >= interval->start) && (interval->right != NULL) && (interval->right->max >= start)) {
			get_intersecting_intervals(interval->right, start, end, tree);
		}
	}
}

/*
 *	Returns values of all intervals that intersect with provided interval.
 *	Additionally it marks all intervals with provided deviation value.
 *	Note:	It is assumed that provided start point is equal to or greater than provided end point.
 */
template <class T>
void IntervalTree<T>::get_intersecting_intervals(int start, int end, int deviation, IntervalTree<T>& tree) {
	get_intersecting_intervals(root, start, end, deviation, tree);
}

/*
 *	Returns values of all intervals that intersect with provided interval.
 *	Additionally it marks all intervals with provided deviation value.
 *	Note:	It is assumed that provided start point is equal to or greater than provided end point.
 */
template <class T>
void IntervalTree<T>::get_intersecting_intervals(Interval<T>* interval, int start, int end, int deviation, IntervalTree<T>& tree) {
	if (interval != NULL) {
		if ((start <= interval->end) && (end >= interval->start)) {
			tree.add_all(interval->start, interval->end, interval->values, deviation);

		}

		if ((interval->left != NULL) && (interval->left->max >= start)) {
			get_intersecting_intervals(interval->left, start, end, deviation, tree);
		}

		if ((end >= interval->start) && (interval->right != NULL) && (interval->right->max >= start)) {
			get_intersecting_intervals(interval->right, start, end, deviation, tree);
		}
	}
}

/* Get all values stored in a tree with respect to their deviations (marked values). */
template <class T>
void IntervalTree<T>::get_marked_values(map<int, vector<T>* >& values) {
	get_marked_values(root, values);
}

/* Get all values stored in a tree with respect to their deviations (marked values) starting with specified. */
template <class T>
void IntervalTree<T>::get_marked_values(Interval<T>* interval, map<int, vector<T>* >& values) {
	if (interval != NULL) {
		typename map<int, vector<T>* >::iterator values_it;

		values_it = values.find(interval->deviation);

		if (values_it != values.end()) {
			(*values_it).second->insert((*values_it).second->end(), interval->values.begin(), interval->values.end());
		}
		else {
			vector<T>* new_values = new vector<T>();
			new_values->insert(new_values->end(), interval->values.begin(), interval->values.end());
			values.insert(pair<int, vector<T>* >(interval->deviation, new_values));
		}

		get_marked_values(interval->left, values);
		get_marked_values(interval->right, values);
	}
}

/* Returns k-first intervals from a tree. */
template <class T>
void IntervalTree<T>::get_inordered(int k, vector<Interval<T>*>& result) {
	if (root != NULL) {
		k = get_inordered(root, k, result);
	}
}

/*
 * Returns k-first intervals from a tree starting with provided interval.
 * Note:	We do not check if provided interval equals to NULL.
 */
template <class T>
int IntervalTree<T>::get_inordered(Interval<T>* interval, int k, vector<Interval<T>*>& result) {
	if (interval->left != NULL) {
		k = get_inordered(interval->left, k, result);
	}

	if (k > 0) {
		k -= 1;
		result.push_back(interval);

		if (interval->right != NULL) {
			k = get_inordered(interval->right, k, result);
		}
	}

	return k;
}

/* Returns k-last intervals from a tree in descending order. */
template <class T>
void IntervalTree<T>::get_inordered_desc(int k, vector<Interval<T>*>& result) {
	if (root != NULL) {
		k = get_inordered_desc(root, k, result);
	}
}

/*
 * Returns k-last intervals from a tree in descending order starting with provided interval.
 * Note:	We do not check if provided interval equals to NULL.
 */
template <class T>
int IntervalTree<T>::get_inordered_desc(Interval<T>* interval, int k, vector<Interval<T>*>& result) {
	if (interval->right != NULL) {
		k = get_inordered_desc(interval->right, k, result);
	}

	if (k > 0) {
		k -= 1;
		result.push_back(interval);

		if (interval->left != NULL) {
			k = get_inordered_desc(interval->left, k, result);
		}
	}

	return k;
}

/* Gets nearest interval to the position from right. */
template <class T>
Interval<T>* IntervalTree<T>::get_nearest_from_right(int position) {
	return get_nearest_from_right(root, position, NULL);
}

/* Gets nearest interval to the position from right starting with specified interval. */
template <class T>
Interval<T>* IntervalTree<T>::get_nearest_from_right(Interval<T>* interval, int position, Interval<T>* nearest) {
	if (interval != NULL) {
		if (interval->start <= position) {
			nearest = get_nearest_from_right(interval->right, position, nearest);
		}
		else {
			nearest = interval;
			nearest = get_nearest_from_right(interval->left, position, nearest);
		}
	}

	return nearest;
}

/* Gets nearest interval to the position from left. */
template <class T>
Interval<T>* IntervalTree<T>::get_nearest_from_left(int position) {
	return get_nearest_from_left(root, position, NULL);
}

/* Gets nearest interval to the position from left starting with specified interval. */
template <class T>
Interval<T>* IntervalTree<T>::get_nearest_from_left(Interval<T>* interval, int position, Interval<T>* nearest) {
	if (interval != NULL) {
		if (interval->start < position) {
			nearest = interval;
			nearest = get_nearest_from_left(interval->right, position, nearest);
		}
		else {
			nearest = get_nearest_from_left(interval->left, position, nearest);
		}
	}

	return nearest;
}

/* Gets k-nearest intervals to the position from right. */
template <class T>
void IntervalTree<T>::get_knearest_from_right(int position, int k, vector<Interval<T>*>& result) {
	Interval<T>* nearest = get_nearest_from_right(position);

	if (nearest != NULL) {
		k = get_knearest_from_right(nearest, k, result);
	}
}

/* Gets k-nearest intervals to the position from right starting with specified interval. */
template <class T>
int IntervalTree<T>::get_knearest_from_right(Interval<T>* interval, int k, vector<Interval<T>*>& result) {
	if (k > 0) {
		k -= 1;
		result.push_back(interval);

		if ((k > 0) && (interval->right != NULL)) {
			k = get_inordered(interval->right, k, result);
		}

		if ((k > 0) && (interval->parent != NULL)) {
			if (interval->parent->left == interval) {
				k = get_knearest_from_right(interval->parent, k, result);
			}
			else {
				while ((interval->parent != NULL) && (interval->parent->left != interval)) {
					interval = interval->parent;
				}

				if (interval->parent != NULL) {
					k = get_knearest_from_right(interval->parent, k, result);
				}
			}
		}
	}

	return k;
}

/* Gets k-nearest intervals to the position from left. */
template <class T>
void IntervalTree<T>::get_knearest_from_left(int position, int k, vector<Interval<T>*>& result) {
	Interval<T>* nearest = get_nearest_from_left(position);

	if (nearest != NULL) {
		k = get_knearest_from_left(nearest, k, result);
	}
}

/* Gets k-nearest intervals to the position from left starting with specified interval. */
template <class T>
int IntervalTree<T>::get_knearest_from_left(Interval<T>* interval, int k, vector<Interval<T>*>& result) {
	if (k > 0) {
		k -= 1;
		result.push_back(interval);

		if ((k > 0) && (interval->left != NULL)) {
			k = get_inordered_desc(interval->left, k, result);
		}

		if ((k > 0) && (interval->parent != NULL)) {
			if (interval->parent->right == interval) {
				k = get_knearest_from_left(interval->parent, k, result);
			}
			else {
				while ((interval->parent != NULL) && (interval->parent->right != interval)) {
					interval = interval->parent;
				}
				if (interval->parent != NULL) {
					k = get_knearest_from_left(interval->parent, k, result);
				}
			}
		}
	}

	return k;
}

/*
 *	Prints tree to output.
 *	Note:	Only for testing purposes.
 */
template <class T>
void IntervalTree<T>::print(ostream& stream) {
	print(stream, root, 0);
}

/* Returns the number of values stored in a tree. */
template <class T>
int IntervalTree<T>::get_values_count() {
	return get_values_count(root, 0);
}

/* Returns the number of values stored in a tree. */
template <class T>
int IntervalTree<T>::get_values_count(Interval<T>* interval, int count) {
	if (interval != NULL) {
		count += interval->values.size();
	}
	else {
		return count;
	}

	count = get_values_count(interval->left, count);
	count = get_values_count(interval->right, count);

	return count;
}


/* Returns the number of distinct intervals in a tree. */
template <class T>
int IntervalTree<T>::get_intervals_count() {
	return get_intervals_count(root, 0);
}

/* Returns the number of distinct intervals in a tree starting at specified node(interval). */
template <class T>
int IntervalTree<T>::get_intervals_count(Interval<T>* interval, int count) {
	if (interval != NULL) {
		count += 1;
	}
	else {
		return count;
	}

	count = get_intervals_count(interval->left, count);
	count = get_intervals_count(interval->right, count);

	return count;
}

/* Returns the height of a tree. */
template <class T>
int IntervalTree<T>::get_height() {
	return get_height(root, 0, 0);
}

/* Calculates the height of a tree starting at specified node(interval). */
template <class T>
int IntervalTree<T>::get_height(Interval<T>* interval, int path_height, int max_height) {
	if (interval != NULL) {
		path_height += 1;
	}
	else {
		return path_height > max_height ? path_height : max_height;
	}

	max_height = get_height(interval->left, path_height, max_height);
	max_height = get_height(interval->right, path_height, max_height);

	return max_height;
}

template <class T>
void IntervalTree<T>::print(ostream& stream, Interval<T>* interval, int intend) {
	if (interval == NULL) {
		stream << "<empty tree>" << endl;
		return;
	}

	if (interval->right != NULL) {
		print(stream, interval->right, intend + 1);
	}

	for (int i = 0; i < intend; i++) {
		stream << "\t";
	}

	if (interval->color == Interval<T>::BLACK) {
		stream << "[" << interval->start << ", " << interval->end << "]:" << interval->values.front() << ", " << interval->max << endl;
	}
	else {
		stream << "<[" << interval->start << ", " << interval->end << "]:" << interval->values.front() << "," << interval->max << ">" << endl;
	}

	if (interval->left != NULL) {
		print(stream, interval->left, intend + 1);
	}
}

template <class T>
void IntervalTree<T>::clear() {
	remove_interval(root);
	root = NULL;
}

/*
 *	Deletes intervals starting from provided one without touching the values.
 *	Note:	Is used by clear method.
 */
template <class T>
void IntervalTree<T>::remove_interval(Interval<T>* interval) {
	if (interval != NULL) {
		remove_interval(interval->right);
		remove_interval(interval->left);

		delete interval;
	}
}

/*
 *	Deletes intervals starting from provided one and clears memory.
 *	Note:	Is used by destructor.
 */
template <class T>
void IntervalTree<T>::erase_interval(Interval<T>* interval) {
	if (interval != NULL) {
		erase_interval(interval->right);
		erase_interval(interval->left);

		delete interval;
	}
}

/*
 *	Deletes intervals starting from provided one and clears memory.
 *	Is specialized for char* (arrays of chars).
 *	Note:	Is used by destructor.
 */
template <>
void IntervalTree<char*>::erase_interval(Interval<char*>* interval) {
	if (interval != NULL) {
		for (unsigned int i = 0; i < interval->values.size(); i++) {
			free(interval->values.at(i));
		}
		interval->values.clear();

		erase_interval(interval->right);
		erase_interval(interval->left);

		delete interval;
	}
}

template class IntervalTree<int>;
template class IntervalTree<char*>;
