import sys, os, re
from setuptools import setup

def get_binaries():
	files = [f for f in os.listdir('bearlibterminal') if 'BearLibTerminal.' in f]
	if not files:
		raise RuntimeError('No library binary in the package')
	return files

def get_version():
	top_line = open('CHANGELOG.md').readline()
	return re.findall('^\#+\s+([\w\.]+)\s+.*$', top_line)[0]

setup(
  name='bearlibterminal',
  version=get_version(),
  description='BearLibTerminal is a pseudoterminal window library',
  long_description=open('PACKAGE.rst', 'r').read(),
  url='http://foo.wyrd.name/en:bearlibterminal',
  author='Alexander Malinin',
  author_email='cfyzium@gmail.com',
  license='MIT',
  packages=['bearlibterminal'],
  zip_safe=False,
  package_data={'bearlibterminal': get_binaries()},
  keywords='roguelike, terminal, ascii, tiles, unicode',
  platforms='Windows, Linux, Mac OS X',
  classifiers=[
    'Development Status :: 4 - Beta',
    'Environment :: Win32 (MS Windows)',
    'Environment :: X11 Applications',
    'Environment :: MacOS X :: Cocoa',
    'Intended Audience :: Developers',
    'License :: OSI Approved :: MIT License',
    'Natural Language :: English',
    'Operating System :: Microsoft :: Windows',
    'Operating System :: POSIX :: Linux',
    'Operating System :: MacOS :: MacOS X',
    'Programming Language :: Python :: 2',
    'Programming Language :: Python :: 3',
    'Topic :: Games/Entertainment',
    'Topic :: Multimedia :: Graphics',
    'Topic :: Software Development :: Libraries'
    ]
)
