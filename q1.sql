select s.name, s.semester, h.matrnr
from studenten s, hoeren h
where s.matrnr =   h.matrnr 
