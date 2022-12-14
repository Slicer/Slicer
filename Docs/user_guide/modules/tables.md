# Tables

## Overview

The Tables module allows displaying and editing of spreadsheets.

## Panels and their use

- **Input**:
  - **Active table**: Select the table node to edit/view.
  - **Lock button**: Allows locking the table node to read-only. Only applies to the user interface. The node can eb still modified programmatically.

- **Edit**:
  - **Copy button**: Copy contents of selected cells to clipboard.
  - **Paste button**: Paste contents of clipboard at the current position. Data that does not fit into the table is ignored.
  - **Add column button**: Add an empty column at the end of the table.
  - **Delete column button**: Delete all selected columns. If any cell is selected in a column the entire column will be removed.
  - **Lock first column button**: Use the first column as row header. Header cannot be edited and is not copied to clipboard but still saved to file.
  - **Add row button**: Add an empty row at the end of the table.
  - **Delete row button**: Delete all selected rows. If any cell is selected in a row the entire row will be removed.
  - **Lock first row button**: Use the first row as column header. Header cannot be edited and is not copied to clipboard but still saved to file.

## Contributors

Andras Lasso (PerkLab), Kevin Wang (PMH)

## Acknowledgements

This work was was partially funded by OCAIRO, the Applied Cancer Research Unit program of Cancer Care Ontario, and Department of Anesthesia and Critical Care Medicine, Children’s Hospital of Philadelphia.
