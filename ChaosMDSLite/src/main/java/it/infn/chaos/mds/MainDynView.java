package it.infn.chaos.mds;

import com.vaadin.annotations.AutoGenerated;
import com.vaadin.ui.Alignment;
import com.vaadin.ui.Button;
import com.vaadin.ui.CustomComponent;
import com.vaadin.ui.HorizontalLayout;
import com.vaadin.ui.Label;
import com.vaadin.ui.Table;
import com.vaadin.ui.VerticalLayout;

public class MainDynView extends CustomComponent {

	/*- VaadinEditorProperties={"grid":"RegularGrid,20","showGrid":true,"snapToGrid":true,"snapToObject":true,"movingGuides":false,"snappingDistance":10} */

	@AutoGenerated
	private VerticalLayout mainLayout;

	@AutoGenerated
	private VerticalLayout verticalLayout_6;

	@AutoGenerated
	private VerticalLayout verticalLayout_5;

	@AutoGenerated
	private HorizontalLayout horizontalLayout_5;

	@AutoGenerated
	private Table tableDatasetAttribute;

	@AutoGenerated
	private VerticalLayout verticalLayout_4;

	@AutoGenerated
	private HorizontalLayout horizontalLayout_4;

	@AutoGenerated
	private Table tableDevice;

	@AutoGenerated
	private HorizontalLayout unitServerHorizontalLayout;

	@AutoGenerated
	private VerticalLayout verticalLayout_2;

	@AutoGenerated
	private HorizontalLayout horizontalLayout_2;

	@AutoGenerated
	private Button DelCU;

	@AutoGenerated
	private Button NewCU;

	@AutoGenerated
	private Table CUTypeTable;

	@AutoGenerated
	private VerticalLayout verticalLayout_1;

	@AutoGenerated
	private HorizontalLayout horizontalLayout_1;

	@AutoGenerated
	private Button buttonDelUS;

	@AutoGenerated
	private Button buttonNewUS;

	@AutoGenerated
	private Table UnitServerTable;

	@AutoGenerated
	private Button buttonPreference;

	@AutoGenerated
	private Label label_1;

	/**
	 * 
	 */
    private static final long       serialVersionUID        = 1722906903909249860L;

	/**
	 * The constructor should first build the main layout, set the
	 * composition root and then do any custom initialization.
	 *
	 * The constructor will not be automatically regenerated by the
	 * visual editor.
	 */
	public MainDynView() {
		buildMainLayout();
		setCompositionRoot(mainLayout);

		// TODO add user code here
	}

	public Table getTableUnitServer() {
		// TODO Auto-generated method stub
		return UnitServerTable;
	}

	public Table getTableUnitServerCUType() {
		// TODO Auto-generated method stub
		return CUTypeTable;
	}

	public Table getTableDevice() {
		// TODO Auto-generated method stub
		return tableDevice;
	}

	public Button getButtonNewUS() {
		// TODO Auto-generated method stub
		return buttonNewUS;
	}

	public Button getButtonDelUS() {
		// TODO Auto-generated method stub
		return buttonDelUS;
	}

	public Button getButtonAddCUType() {
		// TODO Auto-generated method stub
		return NewCU;
	}

	public Button getButtonRemoveCUType() {
		// TODO Auto-generated method stub
		return DelCU;
	}

	public Table getTableDatasetAttribute() {
		// TODO Auto-generated method stub
		return tableDatasetAttribute;
	}

	@AutoGenerated
	private VerticalLayout buildMainLayout() {
		// common part: create layout
		mainLayout = new VerticalLayout();
		mainLayout.setImmediate(false);
		mainLayout.setWidth("100%");
		mainLayout.setHeight("-1px");
		mainLayout.setMargin(false);
		
		// top-level component properties
		setWidth("100.0%");
		setHeight("-1px");
		
		// label_1
		label_1 = new Label();
		label_1.setImmediate(false);
		label_1.setWidth("-1px");
		label_1.setHeight("-1px");
		label_1.setValue("<h1>!CHAOS MetadataServer Lite</h1>");
		label_1.setContentMode(3);
		mainLayout.addComponent(label_1);
		
		// buttonPreference
		buttonPreference = new Button();
		buttonPreference.setCaption("Data Server Setup");
		buttonPreference.setImmediate(true);
		buttonPreference.setDescription("Data Service Setup");
		buttonPreference.setWidth("-1px");
		buttonPreference.setHeight("-1px");
		mainLayout.addComponent(buttonPreference);
		
		// unitServerHorizontalLayout
		unitServerHorizontalLayout = buildUnitServerHorizontalLayout();
		mainLayout.addComponent(unitServerHorizontalLayout);
		
		// verticalLayout_6
		verticalLayout_6 = buildVerticalLayout_6();
		mainLayout.addComponent(verticalLayout_6);
		
		return mainLayout;
	}

	@AutoGenerated
	private HorizontalLayout buildUnitServerHorizontalLayout() {
		// common part: create layout
		unitServerHorizontalLayout = new HorizontalLayout();
		unitServerHorizontalLayout.setImmediate(false);
		unitServerHorizontalLayout.setWidth("100.0%");
		unitServerHorizontalLayout.setHeight("100.0%");
		unitServerHorizontalLayout.setMargin(true);
		
		// verticalLayout_1
		verticalLayout_1 = buildVerticalLayout_1();
		unitServerHorizontalLayout.addComponent(verticalLayout_1);
		
		// verticalLayout_2
		verticalLayout_2 = buildVerticalLayout_2();
		unitServerHorizontalLayout.addComponent(verticalLayout_2);
		unitServerHorizontalLayout.setComponentAlignment(verticalLayout_2,
				new Alignment(6));
		
		return unitServerHorizontalLayout;
	}

	@AutoGenerated
	private VerticalLayout buildVerticalLayout_1() {
		// common part: create layout
		verticalLayout_1 = new VerticalLayout();
		verticalLayout_1.setImmediate(false);
		verticalLayout_1.setWidth("100.0%");
		verticalLayout_1.setHeight("100.0%");
		verticalLayout_1.setMargin(true);
		verticalLayout_1.setSpacing(true);
		
		// UnitServerTable
		UnitServerTable = new Table();
		UnitServerTable.setCaption("Unit Servers");
		UnitServerTable.setImmediate(false);
		UnitServerTable.setWidth("100.0%");
		UnitServerTable.setHeight("100.0%");
		verticalLayout_1.addComponent(UnitServerTable);
		
		// horizontalLayout_1
		horizontalLayout_1 = buildHorizontalLayout_1();
		verticalLayout_1.addComponent(horizontalLayout_1);
		
		return verticalLayout_1;
	}

	@AutoGenerated
	private HorizontalLayout buildHorizontalLayout_1() {
		// common part: create layout
		horizontalLayout_1 = new HorizontalLayout();
		horizontalLayout_1.setImmediate(false);
		horizontalLayout_1.setWidth("-1px");
		horizontalLayout_1.setHeight("-1px");
		horizontalLayout_1.setMargin(false);
		
		// buttonNewUS
		buttonNewUS = new Button();
		buttonNewUS.setCaption("New");
		buttonNewUS.setImmediate(true);
		buttonNewUS.setDescription("Create a New Unit Server");
		buttonNewUS.setWidth("-1px");
		buttonNewUS.setHeight("-1px");
		horizontalLayout_1.addComponent(buttonNewUS);
		
		// buttonDelUS
		buttonDelUS = new Button();
		buttonDelUS.setCaption("Del");
		buttonDelUS.setImmediate(true);
		buttonDelUS.setDescription("Delete the selected US");
		buttonDelUS.setWidth("-1px");
		buttonDelUS.setHeight("-1px");
		horizontalLayout_1.addComponent(buttonDelUS);
		
		return horizontalLayout_1;
	}

	@AutoGenerated
	private VerticalLayout buildVerticalLayout_2() {
		// common part: create layout
		verticalLayout_2 = new VerticalLayout();
		verticalLayout_2.setImmediate(false);
		verticalLayout_2.setWidth("100.0%");
		verticalLayout_2.setHeight("100.0%");
		verticalLayout_2.setMargin(true);
		verticalLayout_2.setSpacing(true);
		
		// CUTypeTable
		CUTypeTable = new Table();
		CUTypeTable.setCaption("Allocated CUs");
		CUTypeTable.setImmediate(false);
		CUTypeTable.setWidth("100.0%");
		CUTypeTable.setHeight("100.0%");
		verticalLayout_2.addComponent(CUTypeTable);
		
		// horizontalLayout_2
		horizontalLayout_2 = buildHorizontalLayout_2();
		verticalLayout_2.addComponent(horizontalLayout_2);
		
		return verticalLayout_2;
	}

	@AutoGenerated
	private HorizontalLayout buildHorizontalLayout_2() {
		// common part: create layout
		horizontalLayout_2 = new HorizontalLayout();
		horizontalLayout_2.setImmediate(false);
		horizontalLayout_2.setWidth("-1px");
		horizontalLayout_2.setHeight("-1px");
		horizontalLayout_2.setMargin(false);
		
		// NewCU
		NewCU = new Button();
		NewCU.setCaption("New");
		NewCU.setImmediate(true);
		NewCU.setWidth("-1px");
		NewCU.setHeight("-1px");
		horizontalLayout_2.addComponent(NewCU);
		
		// DelCU
		DelCU = new Button();
		DelCU.setCaption("Del");
		DelCU.setImmediate(true);
		DelCU.setWidth("-1px");
		DelCU.setHeight("-1px");
		horizontalLayout_2.addComponent(DelCU);
		
		return horizontalLayout_2;
	}

	@AutoGenerated
	private VerticalLayout buildVerticalLayout_6() {
		// common part: create layout
		verticalLayout_6 = new VerticalLayout();
		verticalLayout_6.setImmediate(false);
		verticalLayout_6.setWidth("100.0%");
		verticalLayout_6.setHeight("100.0%");
		verticalLayout_6.setMargin(true);
		
		// verticalLayout_4
		verticalLayout_4 = buildVerticalLayout_4();
		verticalLayout_6.addComponent(verticalLayout_4);
		
		// verticalLayout_5
		verticalLayout_5 = buildVerticalLayout_5();
		verticalLayout_6.addComponent(verticalLayout_5);
		
		return verticalLayout_6;
	}

	@AutoGenerated
	private VerticalLayout buildVerticalLayout_4() {
		// common part: create layout
		verticalLayout_4 = new VerticalLayout();
		verticalLayout_4.setImmediate(false);
		verticalLayout_4.setWidth("100.0%");
		verticalLayout_4.setHeight("100.0%");
		verticalLayout_4.setMargin(true);
		verticalLayout_4.setSpacing(true);
		
		// tableDevice
		tableDevice = new Table();
		tableDevice.setCaption("Registered CU");
		tableDevice.setImmediate(false);
		tableDevice.setDescription("Registered CU");
		tableDevice.setWidth("100.0%");
		tableDevice.setHeight("100.0%");
		verticalLayout_4.addComponent(tableDevice);
		
		// horizontalLayout_4
		horizontalLayout_4 = new HorizontalLayout();
		horizontalLayout_4.setImmediate(false);
		horizontalLayout_4.setWidth("-1px");
		horizontalLayout_4.setHeight("-1px");
		horizontalLayout_4.setMargin(false);
		verticalLayout_4.addComponent(horizontalLayout_4);
		
		return verticalLayout_4;
	}

	@AutoGenerated
	private VerticalLayout buildVerticalLayout_5() {
		// common part: create layout
		verticalLayout_5 = new VerticalLayout();
		verticalLayout_5.setImmediate(false);
		verticalLayout_5.setWidth("100.0%");
		verticalLayout_5.setHeight("100.0%");
		verticalLayout_5.setMargin(true);
		verticalLayout_5.setSpacing(true);
		
		// tableDatasetAttribute
		tableDatasetAttribute = new Table();
		tableDatasetAttribute.setCaption("CU Dataset");
		tableDatasetAttribute.setImmediate(false);
		tableDatasetAttribute.setWidth("100.0%");
		tableDatasetAttribute.setHeight("100.0%");
		verticalLayout_5.addComponent(tableDatasetAttribute);
		
		// horizontalLayout_5
		horizontalLayout_5 = new HorizontalLayout();
		horizontalLayout_5.setImmediate(false);
		horizontalLayout_5.setWidth("-1px");
		horizontalLayout_5.setHeight("-1px");
		horizontalLayout_5.setMargin(false);
		verticalLayout_5.addComponent(horizontalLayout_5);
		
		return verticalLayout_5;
	}

}
