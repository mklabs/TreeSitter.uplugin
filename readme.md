# TreeSitter.uplugin

> Unreal Engine plugin that integrates the [tree-sitter](https://tree-sitter.github.io) library as a third-party module for in-editor use.

This is a work in progress, but the third-party module bringing access to tree-sitter API within Unreal is there and working.

See `Source/TreeSitter/Private/Tests/TreeSitterParser.spec.cpp` for usage examples.

## Introduction

The primary use case I'm going for is markdown related, aiming to implement native markdown to slate rendering for usage in the editor (and possibly UMG).

The other one I have in mind is syntax highlighting for any language tree-sitter have parsers for.

Tree-sitter is a very nice piece of technology. Being able to parse almost any language and run queries against the Abstract Syntax Tree (AST) is powerful in itself. Its use cases and possible applications are quite
extensive.


## Features

- **Tree-sitter as Third-Party Module**:
  - Windows support only (for now).
- **Tree-sitter Playground**:
  - Slate widget replicating the functionnality of neovim treesitter playground (:InspectTree) as close as possible. A web version is available there: https://tree-sitter.github.io/tree-sitter/7-playground.html
  - Paste code on the left, see AST on the right.
  - Dropdown menu to select language (e.g., JavaScript, CSS, Python).

### Playground

1. Run `TreeSitter.Playground` in the console command promt (below the output log)
2. Paste code in the left panel.
3. Select a language from the dropdown.
4. View the AST in the right panel.

## Setup

From the root of your unreal project.

1. Clone this repository into `Plugins`:
   ```bash
   git clone https://github.com/mklabs/TreeSitter.uplugin.git Plugins/TreeSitter
   ```
2. When installed locally, plugins are enabled by default.
    - If that's not the case, enable the plugin under **Edit → Plugins** in the Editor.
3. Build the project.

Static libraries for tree-sitter and DLLs for language parsers are included as part of the repository for conveniency (and only for Win64 for now).

## Usage

Ensure you have `TreeSitter` and `TreeSitterLibrary` module dependency defined in your Build.cs file.

```cpp
#include "TreeSitterParser.h"
#include "tree_sitter/api.h"

// Create the parser
TSharedPtr<FTreeSitterParser> Parser = MakeShared<FTreeSitterParser>();

// Set parser language
Parser->SetLanguage(ETreeSitterLanguage::Json);

// Provide some source code
const FString SourceCode = "[1, null]";

// Parse the code
TSTree* Tree = Parser->Parse(SourceCode);

// Get the root node of the syntax tree.
const TSNode RootNode = ts_tree_root_node(Tree);

// Get some child nodes.
const TSNode ArrayNode = ts_node_named_child(RootNode, 0);
const TSNode NumberNode = ts_node_named_child(ArrayNode, 0);

// Check that the nodes have the expected types.
check(strcmp(ts_node_type(RootNode), "document") == 0);
check(strcmp(ts_node_type(ArrayNode), "array") == 0);
check(strcmp(ts_node_type(NumberNode), "number") == 0);

// Check that the nodes have the expected child counts.
check(ts_node_child_count(RootNode) == 1);
check(ts_node_child_count(ArrayNode) == 5);
check(ts_node_named_child_count(ArrayNode) == 2);
check(ts_node_child_count(NumberNode) == 0);

// Clean up tree
ts_tree_delete(Tree);
```
---

## Current Limitations

- **Windows only**.
- Limited language support (dropdown options). Only testing with a few parsers for now (JavaScript, JSON, Markdown, Markdown inline).

## Todo

**Playground**

- [ ] Syntax highlighting for left pane
- [ ] Query Editor
- [ ] Options to:
    - Toggle display of anonymous nodes
    - Toggle the display of source language
    - Press `<Enter>`  to jump to node under the cursor
    - Highlights code corresponding to node under the cursor

**Misc**

- Load remaning of parsers found in `Source/ThirdParty/TreeSitterLibrary/Win64/languages`

## Credits

Big thanks to:

- [Tree-sitter](https://tree-sitter.github.io) for the powerful parsing magic.
- [Georgy Treshchev](https://github.com/gtreshchev) for this fantastic article: https://georgy.dev/posts/third-party-integration
- [Iku Iwasa](https://github.com/iquiw) for https://github.com/iquiw/emacs-tree-sitter-module-dll
    - Most of the DLLs for language parsers are coming from this repository ([workflow used to build the DLLs](https://github.com/iquiw/emacs-tree-sitter-module-dll/blob/main/.github/workflows/build.yml), uploaded as build artifacts on the [release page](https://github.com/iquiw/emacs-tree-sitter-module-dll/releases)).
    - Only markdown and markdown-inline parsers were manually compiled.

---

## License

This project is licensed under MIT. 

Third party:

- [Tree-sitter](./Source/ThirdParty/TreeSitterLibrary/LICENSE)
- [Language Parsers](./Source/ThirdParty/TreeSitterLibrary/Win64/languages/licenses)
