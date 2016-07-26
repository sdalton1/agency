import re
import os
import pandas as pd
import matplotlib.pyplot as plt
from StringIO import StringIO
from matplotlib import rc

rc('font', **{'family':'serif', 'serif':['Computer Modern']})
rc('text', usetex=True);
fontsize = 18;
family = 'sans-serif';

def save_plot(pdfname):
    plt.savefig(pdfname)
    os.system('pdfcrop {0} {0}'.format(pdfname)); # whitespace crop
    os.system('pdfcrop --margins \'0 2 0 0\' {0} {0}'.format(pdfname)); # top tickmark crop

filename_template = 'hpgmg-fv-{}.log';

names = ['openmp', 'agency', 'agency-openmp'];
data  = {};

for name in names:
    with open(filename_template.format(name)) as f:
        read_data  = f.read();
        matches    = re.findall(r'Breakdown.*?\n{3}(.*?)\n{5}', read_data, re.DOTALL);
        block      = re.search(r'- \n(.*?)\n-', matches[0], re.DOTALL).group(1);
        stripblock = re.sub(r'([A-Za-z])\s([A-Za-z])', r'\1\2', block);

        df = pd.read_table(StringIO(stripblock), delim_whitespace=True, header=None);
        df.columns = ['Level'] + ['{}'.format(i) for i in range(len(df.columns) - 2)] + ['Total'];
        df = df.set_index('Level');
        data[name] = df.T;

        columns=['smooth', 'residual', 'BLAS1', 'BoundaryConditions', 'Restriction', 'Interpolation', 'GhostZoneExchange'];
        M = data[name].as_matrix(columns);

        plt.figure();
        lineObjects = plt.semilogy(M[:-1,:] * 100, '-o');
        plt.ylabel('Level');
        plt.ylabel('Time (ms)');
        plt.legend(iter(lineObjects), columns, loc='lower center', bbox_to_anchor=(0.5, -0.10), ncol=4, fancybox=True, shadow=True);
        plt.title(name);
        save_plot('{}_figure.pdf'.format(name));

plt.show();
