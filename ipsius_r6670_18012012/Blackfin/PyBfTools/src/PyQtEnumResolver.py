
import PyQt4.QtCore


if __name__ == "__main__":
    
    m = PyQt4.QtCore.QProcess.staticMetaObject
    
    print(m)
    print(m.className())
    print(m.enumeratorCount())
    
    for i in range(m.enumeratorCount()):
        e = m.enumerator(i)
        print(e.name())
        
        