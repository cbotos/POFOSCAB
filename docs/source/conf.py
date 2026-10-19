# Configuration file for the Sphinx documentation builder.
#
# For the full list of built-in configuration values, see the documentation:
# https://www.sphinx-doc.org/en/master/usage/configuration.html

import os
import re

# -- Version: parsed from src/pftd/version.inc (hand-maintained, see that
# file's own header) - never import it as Python, there is no Python
# package here.
_version_inc = os.path.abspath(
    os.path.join(os.path.dirname(__file__), '../../src/pftd/version.inc')
)
with open(_version_inc) as f:
    _version_src = f.read()


def _version_field(name):
    m = re.search(rf'{name}\s+equ\s+(\d+)', _version_src)
    return m.group(1)


package_version = '.'.join(
    _version_field(f'VERSION_{part}') for part in ('MAJOR', 'MINOR', 'PATCH')
)

# -- Project information -----------------------------------------------------
# https://www.sphinx-doc.org/en/master/usage/configuration.html#project-information

project = 'pofoscab'
copyright = '2026, Petr Kracik'
author = 'Petr Kracik'
version = package_version
release = package_version

# -- General configuration ---------------------------------------------------
# https://www.sphinx-doc.org/en/master/usage/configuration.html#general-configuration

extensions = [
    'sphinx.ext.githubpages',
    'myst_parser',
]

templates_path = ['_templates']
exclude_patterns = []

# -- Options for HTML output -------------------------------------------------
# https://www.sphinx-doc.org/en/master/usage/configuration.html#options-for-html-output

# Try to use ReadTheDocs theme, fallback to pyramid/classic if not available
try:
    import sphinx_rtd_theme
    html_theme = 'sphinx_rtd_theme'
    html_theme_options = {
        'navigation_depth': 5,
        'collapse_navigation': False,
        'sticky_navigation': True,
        'includehidden': True,
        'titles_only': False,
        'prev_next_buttons_location': 'both',
        'style_external_links': False,
    }
except ImportError:
    html_theme = 'pyramid'  # Better fallback theme - clean and modern

html_static_path = ['_static']
html_title = 'POFOSCAB Documentation'
html_short_title = 'pofoscab'

# MyST parser configuration
myst_enable_extensions = [
    "colon_fence",      # ::: fence syntax
    "deflist",          # Definition lists
    "tasklist",         # Task lists [ ]
]
myst_heading_anchors = 4  # Generate anchors for h1-h4 (protocol.md uses #### for subsections)

# Force global TOC to always show in sidebar (fixes navigation disappearing on API Reference)
html_sidebars = {
    '**': [
        'globaltoc.html',
        'relations.html',
        'sourcelink.html',
        'searchbox.html',
    ]
}
