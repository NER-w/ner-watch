# Mastering Zephyr Driver Development

The main repository of NER-Watch. 

## Getting Started

Zephyr sdk should be installed prior to the next steps.

### Initialization

The first step is to initialize the workspace folder where the
`ner-watch` and needed Zephyr modules will be cloned.

```shell
# initialize workspace for the ner-watch
west init -m https://github.com/ --mr main ner-watch
# update Zephyr modules
cd ner-watch
west update
```

### Build & Run

The application can be built by running:

```shell
west build -b esp32 -s app --sysbuild
```

```shell
west flash
```
