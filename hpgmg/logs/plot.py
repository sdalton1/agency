import re
import pandas as pd
import matplotlib.pyplot as plt
from StringIO import StringIO

filename_template = 'hpgmg-fv-{}.log';

names = ['openmp', 'agency'];
data  = {};

for name in names:
    with open(filename_template.format(name)) as f:
        read_data  = f.read();
        matches    = re.findall(r'Breakdown.*?\n{3}(.*?)\n{5}', read_data, re.DOTALL);
        block      = re.search(r'- \n(.*?)\n-', matches[0], re.DOTALL).group(1);
        stripblock = re.sub(r'([A-Za-z])\s([A-Za-z])', r'\1\2', block);

        df = pd.read_table(StringIO(stripblock), delim_whitespace=True, header=None);
        df.columns = ['Operation'] + ['level{}'.format(i) for i in range(len(df.columns) - 1)];
        data[name] = df.T;

# data[names[1]].iloc[2:-1][1:-1].plot(style='-o');
print data[names[0]][0];
# plt.plot(df[names[0]].BLAS1);
# plt.show();
