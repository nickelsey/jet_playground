from sklearn.preprocessing import PolynomialFeatures
from sklearn.preprocessing import StandardScaler
from sklearn.preprocessing import Imputer
from sklearn.preprocessing import LabelEncoder
from sklearn.preprocessing import OneHotEncoder
from sklearn.preprocessing import LabelBinarizer
from sklearn.pipeline import Pipeline, FeatureUnion



def build_transform( poly_order=1 ):
    pipeline = Pipeline( [('polynomial features', PolynomialFeatures(degree=poly_order))] )
    return pipeline
