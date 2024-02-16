# Configuration file for the Sphinx documentation builder.
#
# For the full list of built-in configuration values, see the documentation:
# https://www.sphinx-doc.org/en/master/usage/configuration.html

# -- Project information -----------------------------------------------------
# https://www.sphinx-doc.org/en/master/usage/configuration.html#project-information

project = 'Cygnus Reach Embedded Stack'
copyright = '2024, Chuck Peplinski, Peter S. Jamrozinski'
author = 'Chuck Peplinski, Peter S. Jamrozinski'

# -- General configuration ---------------------------------------------------
# https://www.sphinx-doc.org/en/master/usage/configuration.html#general-configuration

extensions = [
    'breathe',
    # Include 'myst_parser' if using Markdown
    'myst_parser',
]

templates_path = ['__templates']
exclude_patterns = []

# doxygen xml key:path binding for breath
breathe_projects = { "source_docs": "../__xml" }



# -- Options for HTML output -------------------------------------------------
# https://www.sphinx-doc.org/en/master/usage/configuration.html#options-for-html-output

html_theme = 'alabaster'
html_static_path = ['_static']
