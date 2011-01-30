import javax.swing.AbstractListModel;


public abstract class JythonAbstractListModel extends AbstractListModel {
    public JythonAbstractListModel() {
    super();
    }


    public void fireContentsChanged(Object source, int index0, int index1) {
    super.fireContentsChanged(source, index0, index1);
    }

    public void fireIntervalAdded(Object source, int index0, int index1) {
    super.fireIntervalAdded(source, index0, index1);
    }

    public void fireIntervalRemoved(Object source, int index0, int index1) {
    super.fireIntervalRemoved(source, index0, index1);
    }
}
