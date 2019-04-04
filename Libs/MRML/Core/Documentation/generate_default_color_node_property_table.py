#
#  Program: 3D Slicer
#
#  Copyright (c) Kitware Inc.
#
#  See COPYRIGHT.txt
#  or http://www.slicer.org/copyright/copyright.txt for details.
#
#  Unless required by applicable law or agreed to in writing, software
#  distributed under the License is distributed on an "AS IS" BASIS,
#  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
#  See the License for the specific language governing permissions and
#  limitations under the License.
#
#  This file was originally developed by Jean-Christophe Fillion-Robin, Kitware Inc.
#  and was partially funded by NIH grant 1U24CA194354-01
#

"""
This script allows to generate the markdown table displayed in doxygen
documentation of vtkMRMLColorLogic::AddDefaultColorNodes()
"""
from __future__ import print_function

nodes = slicer.mrmlScene.GetNodesByClass("vtkMRMLColorNode")
nodes.UnRegister(slicer.mrmlScene)

template = "/// | {family} | {category} | {_type} | {node_name} | {singleton_tag} | {node_id} |"

table = []
for index in range(nodes.GetNumberOfItems()):
    n = nodes.GetItemAsObject(index)
    table.append({
        'family': n.GetClassName().replace('vtkMRML', '').replace('Node', ''),
        'category': n.GetAttribute("Category"),
        '_type': n.GetTypeAsString(),
        'node_name': n.GetName(),
        'singleton_tag': n.GetSingletonTag(),
        'node_id': n.GetID()})

titles = {'family': 'Family',
          'category': 'Category',
          '_type': 'Type',
          'node_name': 'Node name',
          'singleton_tag': 'Singleton Tag',
          'node_id': 'Node ID'}
max_row_widths = {column_name: len(column_title) for (column_name, column_title) in titles.items()}

for row in table:
    for column_name in max_row_widths.keys():
        column_width = len(str(row[column_name]))
        if column_width > max_row_widths[column_name]:
            max_row_widths[column_name] = column_width

# Update template with widths
for (column_name, column_width) in max_row_widths.items():
    template = template.replace(column_name, column_name + ":%d" % column_width)

# Print headers
print(template.format(**titles))

# Print separator
print(template.format(**{column_name: '-'*column_width for column_name, column_width in max_row_widths.items()}))

# Print content
for row in table:
    print(template.format(**row) )
