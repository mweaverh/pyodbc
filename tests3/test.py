
from testutils import *
add_to_path()

import pyodbc

con = pyodbc.connect("DSN=pgtests")
print('con:', con)

con.execute("DROP TABLE IF EXISTS BAR")
con.execute("CREATE TABLE BAR(BAR_ID  DECIMAL(18,0) not null primary key)")
# con.execute("ALTER TABLE BAR ADD CONSTRAINT test_bar_constraint_primary PRIMARY KEY(BAR_ID) ENABLE;")

con.execute("insert into bar values(1)")

# No constraint violation
con.execute("insert into bar values(?)", (1,))
print(con.execute("select * from bar").fetchall())

# Force Commit to get ExaSolution driver  constraint violation
cursor = con.execute("insert into bar values(?)", (1,))
cursor.commit()
