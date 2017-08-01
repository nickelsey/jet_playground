import sys
import os
import re
import ROOT
import numpy as np
import pandas
import hashlib
import array

from sklearn.model_selection import GridSearchCV
from sklearn.ensemble import RandomForestRegressor

''' function for parsing the input file to define the set of analysis variables,
  and to generate the input string'''

class analysis_settings():
    alg       = "antikt"
    R         = "0.4"
    naive     = "0"
    inclusive = "0"
    charged   = "0"

def match_settings( set1, set2 ):
  if set1.R != set2.R:
    return False
  if set1.alg != set2.alg:
    return False
  if set1.naive != set2.naive:
    return False
  if set1.inclusive != set2.inclusive:
    return False
  if set1.charged != set2.charged:
    return False
  return True


def parse_root_file( file_name ):
    current_settings = analysis_settings()
    if file_name[-5:] != ".root":
        return False
    file_name = file_name[:-5]

    args = file_name.split("_")
    args[:] = [x for x in args if x != "_"]
    
    ## first, check for valid algorithm
    if args[0] == "antikt" or args[0] == "kt" or args[0] == "CA":
        current_settings.alg = args[0]

    for i in range( len(args) ):
        if args[i] == "R":
          current_settings.R = args[i+1]
        if args[i] == "inc":
          current_settings.inclusive = args[i+1]
        if args[i] == "charged":
          current_settings.charged = args[i+1]
        if args[i] == "naive":
          current_settings.naive = args[i+1]

    return current_settings




## assumes the first file properly formatted file is the
## correct analysis - in other words, if you pass a bunch of files
## with different jetfinding settings, the first one to be parsed
## will invalidate any files with different settings
def validateFiles( files ):
    good_files = []
    nominal_settings = False
    for file in files:
        if os.path.isfile( file ):
            filename = os.path.basename(file)
            file_settings = parse_root_file( filename )
            if nominal_settings == False and file_settings != False:
                nominal_settings = file_settings
                good_files.append( file )
            elif nominal_settings != False and file_settings != False:
                if match_settings( nominal_settings, file_settings ):
                    good_files.append( file )
    return good_files


## used to select branches from the tree
def get_matching_variables(file, tree, patterns):
    from fnmatch import fnmatch
    from root_numpy import list_branches
    
    branches = list_branches(file, tree)
    
    selected = []
    
    for p in patterns:
      for b in branches:
        if fnmatch(b, p) and not b in selected:
          selected.append(b)
    return selected


## used to load the selected patterns from the tree into a pandas dataframe
def load_root_tree(files, tree=None, columns=None, ignore=None, *kargs, **kwargs):
  
    from pandas import DataFrame
    from root_numpy import root2array, list_trees, list_branches
    
    # check if we get a list or a single file
    if not isinstance(files, list):
        files = [files]
    # use the first file to define tree & branches
    init_file = files[0]

    # check to see if there is a specified tree, if not,
    # look for a single tree. If the choice is ambiguous,
    # raise an error and exit
    if tree == None:
      trees = list_trees(init_file)
      if len(trees) == 1:
          tree = trees[0]
      elif len(trees) == 0:
          raise ValueError('Error: no trees found in {}'.format(init_file))
      else:
          raise ValueError('Ambiguous call: more than one tree found in {}'.format(init_file))

    branches = list_branches(init_file, tree)
    
    # match existing branches to branches asked for by user
    if not columns:
        all_vars = branches
    else:
        all_vars = get_matching_variables( branches, columns )

    # handle branches that are asked to be ignored
    if ignore:
        if isinstance( ignore, string_types ):
            ignore = [ignore]
        ignored = get_matching_variables( ignore, branches )
        for var in ignored:
            all_vars.remove(var)

    arr = root2array( files, tree, all_vars, *kargs, **kwargs )

    if 'index' in arr.dtype.names:
        df = DataFrame.from_records(arr, index='index')
    else:
        df = DataFrame.from_records(arr)
    return df

def train_forest( X_train, y_train ):
  param_grid = [ {'n_estimators': [3, 6, 10, 12, 15, 30], 'max_features' : [1, 3, 10, 20 ]},
                {'bootstrap': [False], 'n_estimators': [3, 6, 10, 12, 15, 30], 'max_features': [1, 3, 10, 20] } ]
  forest_reg = RandomForestRegressor()
  grid_search = GridSearchCV(forest_reg, param_grid, cv=5, scoring='neg_mean_squared_error')
  grid_search.fit( X_train, y_train )
  print(grid_search.best_estimator_)
  cvres = grid_search.cv_results_
  for mean_score, params in zip(cvres["mean_test_score"], cvres["params"]):
    print(np.sqrt(-mean_score), params)
  return grid_search.best_estimator_


''' builds a TTree of results and writes them to a TFile '''
def write_to_file( df, tree_name="result_tree", file_name="tmp.root" ):
    out_file = ROOT.TFile( 'test.root', 'recreate' )
    tree = ROOT.TTree( tree_name, 'trained model output' )

    ## get a list of the columns to be written
    branch_names = df.columns.values.tolist()
    branch_address = np.ndarray(shape=(len(branch_names), 1), dtype=np.float64 )
    
    ## set the branches to the tree
    for i in range(len(branch_names)):
        tree.Branch( branch_names[i], branch_address[i], branch_names[i]+"/D" )
    
    ## fill
    for i in range(df.shape[0]):
      for j in range(len(branch_names)):
        branch_address[j][0] = df[branch_names[j]].iloc[i]
      tree.Fill()

    tree.Write()
    out_file.Close()




