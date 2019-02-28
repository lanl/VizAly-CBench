#! /usr/bin/env python
""" Plots distributions of hashed GenericIO files.
"""

import argparse
import gioSqlite as gio_sqlite
import matplotlib.pyplot as plt
import numpy
import os

def operate(y, ref, operation):
    """ Suite of math operations for plot.
    """
    if operation == "none":
        return y
    elif operation == "diff":
        return y - ref
    elif operation == "ratio":
        return y / ref
    else:
        raise NotImplementedError("Do not understand operation {}!".format(operation))

def load_sqlite_data(path, query, sqlite_file):
    """ Loads data using SQLite query.
    """

    # load file
    print("Reading {}...".format(path))
    query_mgr = gio_sqlite.GioSqlite3()
    query_mgr.loadGIOSqlite(sqlite_file)

    # load data
    i = 0
    table_name = "foo_{}".format(i)
    query_mgr.createTable(table_name, (path))

    # execute query
    query = query.replace("__TABLE__", table_name)
    result = query_mgr.runQueryOutputList(query)

    # typecast
    result = numpy.array(result)

    return result

def snap(x, d=0.25):
    """ Snaps data to a grid.
    """
    return numpy.ceil(x / d) * d

# parse command line
parser = argparse.ArgumentParser()
parser.add_argument("--input-file", required=True)
parser.add_argument("--reference-file", required=True)
parser.add_argument("--output-file")
parser.add_argument("--sqlite-file", default="/home/cmbiwer/src/VizAly-Vis_IO/genericio/frontend/GenericIOSQLite.so")
parser.add_argument("--parameters", nargs="+", default=["fof_halo_center_x", "fof_halo_center_y", "fof_halo_center_z", "fof_halo_mass"])
parser.add_argument("--match-parameters", nargs="+", default=["fof_halo_center_x"])
parser.add_argument("--match-parameters-snaps", nargs="+", type=float, default=[-1])
parser.add_argument("--order-by")
parser.add_argument("--descending", action="store_true")
parser.add_argument("--limit", type=float, default=None)
parser.add_argument("--operation", choices=["none", "diff", "ratio"], default="none")
parser.add_argument("--plot-type", choices=["histogram", "scatter"], default="histogram")
parser.add_argument("--bins", type=int, default=50)
opts = parser.parse_args()

# add parameters
for p in opts.match_parameters:
    if p not in opts.parameters:
        opts.parameters.append(p)

# create query
query = "select {} from __TABLE__".format(", ".join(opts.parameters))
if opts.order_by:
    query += " ORDER BY {}".format(opts.order_by)
if opts.descending:
    query += " DESC"
if opts.limit:
    query += " LIMIT {}".format(opts.limit)
print("Will execute the following query: {}".format(query))

# count scalar fields to query and match
n_parameters = len(opts.parameters)
assert(len(opts.match_parameters) == len(opts.match_parameters_snaps))

# determine index of match parameters
match_parameters_idxs = []
for p in opts.match_parameters:
    match_parameters_idxs.append(opts.parameters.index(p))

# set data to compare
if opts.input_file and opts.reference_file:
    data_1 = load_sqlite_data(opts.input_file, query, opts.sqlite_file)
    data_2 = load_sqlite_data(opts.reference_file, query, opts.sqlite_file)
    data_1 = numpy.transpose(data_1)
    data_2 = numpy.transpose(data_2)
else:
    raise KeyError("Must use both --input-file and --reference-file!")

# print statment
print("Read {} particles from {}...".format(data_1.shape[1], opts.input_file))
print("Read {} particles from {}...".format(data_2.shape[1], opts.reference_file))

# snap
data_1_snap = []
data_2_snap = []
for i, d in zip(match_parameters_idxs, opts.match_parameters_snaps):
    if d == -1:
        data_1_snap.append(data_1[i, :])
        data_2_snap.append(data_2[i, :])
    else:
        data_1_snap.append(snap(data_1[i, :], d))
        data_2_snap.append(snap(data_2[i, :], d))
data_1_snap = numpy.vstack(data_1_snap)
data_2_snap = numpy.vstack(data_2_snap)

# transpose for loop
data_1_snap = numpy.transpose(data_1_snap)
data_2_snap = numpy.transpose(data_2_snap)
data_1_snap[data_1_snap == -0.0] = 0.0
data_2_snap[data_2_snap == -0.0] = 0.0

# hash
n_lost = 0
metrics = []
found_1 = []
found_2 = []
lost_1 = []
found_2_idxs = []
for i, d_1 in enumerate(data_1_snap):

    # find matches
    idx = numpy.flatnonzero((d_1 == data_2_snap).all(1))

    # ensure unqiue matches
    # record particles with and without match
    if idx.size > 1:
        raise KeyError("More than one particle matched! Not in good regime!")
    elif idx.size == 0:
        n_lost += 1
        lost_1.append(data_1[:, i])
        continue
    else:
        idx = idx[0]

    # get data values
    r_1 = data_1[:, i]
    r_2 = data_2[:, idx]

    # compute operation
    metric = operate(r_1, r_2, opts.operation)

    # append result
    metrics.append(metric)
    found_1.append(r_1)
    found_2.append(r_2)
    found_2_idxs.append(idx)

# get missed values
mask = numpy.ones(data_2.shape[1], numpy.bool)
mask[found_2_idxs] = 0
lost_2 = numpy.transpose(data_2[:, mask])
if len(lost_1):
    lost_1 = numpy.vstack(lost_1)

# typecast and count
metrics = numpy.vstack(metrics)
found_1 = numpy.vstack(found_1)
found_2 = numpy.vstack(found_2)
n_found = metrics.shape[0]
n_particles = data_1.shape[1]

# print statement
print("Number of matched particles: {}".format(n_found))
print("Number of missed particles: {}".format(n_lost))
assert(n_found + n_lost == n_particles)
print("Missed values:", lost_1)
print("Missed values:", lost_2)

# plot
for i in range(n_parameters):
    p = opts.parameters[i]
    l1 = os.path.basename(opts.input_file).strip("__")
    l2 = os.path.basename(opts.reference_file).strip("__")
    l3 = opts.operation + " of " if opts.operation != "none" else ""

    # plot histogram
    if opts.plot_type == "histogram":
        if opts.operation == "none":
            plt.hist(found_1[:, i], bins=opts.bins, histtype="step", label=l1)
            plt.hist(found_2[:, i], bins=opts.bins, histtype="step", label=l2)
            plt.xlabel(p)
            plt.legend()
        else:
            plt.hist(metrics[:, i], bins=opts.bins)
            plt.xlabel("{}{}".format(l3, p))
        plt.ylabel("Counts")

    # plot scatter
    elif opts.plot_type == "scatter":
        if opts.operation == "none":
            plt.scatter(found_1[:, i], found_2[:, i])
            plt.ylabel("{} of {}".format(p, l1))
            plt.xlabel("{} of {}".format(p, l2))
        else:
            raise NotImplementedError("Cannot use {} with --plot-type scatter!".format(opts.operation))

    # format plot
    plt.grid(zorder=1)

    # display
    if opts.output_file and i == 0:
        if n_parameters > 1:
            print("Will only write out {} into {}!".format(opts.parameters[i], opts.output_file))
        plt.savefig(opts.output_file)
        break
    else:
        plt.show()
    plt.close()
