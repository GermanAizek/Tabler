# Tabler
Tabler is a program that generates nice looking text tables from delimited data.

## Example
### Input:
```
Type	Description	Translation
Campanelle	Small cones	little bells
Casarecci	Short lengths rolled into an S
Cavatelli	Seashell shaped with rolled...
Conchiglie	Seashell shaped	shells
Conchiglioni	Large, stuffable seashell shaped	large shells
Lumaconi	Jumbo Lumache
Maltagliati	Flat roughly cut triangles	badly cut
Orecchiette	Bowl or ear shaped pasta	little ears
Pipe	Larger versions of macaroni
```

Run through `tabler -F '\t' -i`

### Output:
```
+--------------+----------------------------------+--------------+
| Type         | Description                      | Translation  |
+--------------+----------------------------------+--------------+
| Campanelle   | Small cones                      | little bells |
| Casarecci    | Short lengths rolled into an S   |              |
| Cavatelli    | Seashell shaped with rolled...   |              |
| Conchiglie   | Seashell shaped                  | shells       |
| Conchiglioni | Large, stuffable seashell shaped | large shells |
| Lumaconi     | Jumbo Lumache                    |              |
| Maltagliati  | Flat roughly cut triangles       | badly cut    |
| Orecchiette  | Bowl or ear shaped pasta         | little ears  |
| Pipe         | Larger versions of macaroni      |              |
+--------------+----------------------------------+--------------+
```
