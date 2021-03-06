package it.infn.chaos.mds;

import com.vaadin.annotations.AutoGenerated;
import com.vaadin.ui.Alignment;
import com.vaadin.ui.Button;
import com.vaadin.ui.CheckBox;
import com.vaadin.ui.CustomComponent;
import com.vaadin.ui.HorizontalLayout;
import com.vaadin.ui.Label;
import com.vaadin.ui.NativeSelect;
import com.vaadin.ui.Table;
import com.vaadin.ui.TextArea;
import com.vaadin.ui.TextField;
import com.vaadin.ui.VerticalLayout;

public class NewUSCUAssociationVaadin extends CustomComponent {

	/*- VaadinEditorProperties={"grid":"RegularGrid,20","showGrid":true,"snapToGrid":true,"snapToObject":true,"movingGuides":false,"snappingDistance":10} */

	@AutoGenerated
	private VerticalLayout mainLayout;
	@AutoGenerated
	private HorizontalLayout horizontalLayout_4;
	@AutoGenerated
	private Button buttonCancel;
	@AutoGenerated
	private Button buttonSave;
	@AutoGenerated
	private HorizontalLayout horizontalLayout_5;
	@AutoGenerated
	private VerticalLayout verticalLayout_4;
	@AutoGenerated
	private HorizontalLayout horizontalLayout_8;
	@AutoGenerated
	private Button buttonRemAttr;
	@AutoGenerated
	private Button buttonAddAttr;
	@AutoGenerated
	private Table table_attributes;
	@AutoGenerated
	private HorizontalLayout horizontalLayout_3;
	@AutoGenerated
	private VerticalLayout verticalLayout_2;
	@AutoGenerated
	private HorizontalLayout horizontalLayout_6;
	@AutoGenerated
	private Button buttonRemoveDriverSpec;
	@AutoGenerated
	private Button buttonAddDriverSpec;
	@AutoGenerated
	private Table tableDriverSpecifications;
	@AutoGenerated
	private HorizontalLayout horizontalLayout_2;
	@AutoGenerated
	private CheckBox checkBoxAutoLoad;
	@AutoGenerated
	private TextArea cuParamTextArea;
	@AutoGenerated
	private HorizontalLayout horizontalLayout_1;
	@AutoGenerated
	private TextField cuIDTextField;
	@AutoGenerated
	private Label cuIDLabel;
	@AutoGenerated
	private HorizontalLayout horizontalLayoutCUType;
	@AutoGenerated
	private Button cuNewClass;
	@AutoGenerated
	private NativeSelect cuTypeSelection;
	@AutoGenerated
	private HorizontalLayout horizontalLayoutUS;
	@AutoGenerated
	private Label usSelected;
	@AutoGenerated
	private Label usLabel;
	public Table getTableDriverSpecifications() {
		return tableDriverSpecifications;
	}

	public Button getButtonRemoveDriverSpec() {
		return buttonRemoveDriverSpec;
	}

	public Button getButtonAddDriverSpec() {
		return buttonAddDriverSpec;
	}

	public TextArea getCuParamTextArea() {
		return cuParamTextArea;
	}

	public TextField getCuIDTextField() {
		return cuIDTextField;
	}

	public NativeSelect getCuTypeSelection() {
		return cuTypeSelection;
	}

	public Label getUsSelected() {
		return usSelected;
	}

	public Button getButtonCancel() {
		return buttonCancel;
	}

	public Button getButtonSave() {
		return buttonSave;
	}

	public CheckBox getCheckBoxAutoLoad() {
		return checkBoxAutoLoad;
	}

	/**
	 * The constructor should first build the main layout, set the composition root and then do any custom initialization.
	 * 
	 * The constructor will not be automatically regenerated by the visual editor.
	 */
	public NewUSCUAssociationVaadin() {
		buildMainLayout();
		setCompositionRoot(mainLayout);

		// TODO add user code here
	}

	public Table getTableAttributeConfig() {
		// TODO Auto-generated method stub
		return table_attributes;
	}

	public Button getButtonAddAttributeConfig() {
		// TODO Auto-generated method stub
		return buttonAddAttr;
	}

	public Button getButtonRemoveAttributeConfig() {
		// TODO Auto-generated method stub
		return buttonRemAttr;
	}

	public Button getButtonNewCUClass() {
		// TODO Auto-generated method stub
		return cuNewClass;
	}

	

	@AutoGenerated
	private VerticalLayout buildMainLayout() {
		// common part: create layout
		mainLayout = new VerticalLayout();
		mainLayout.setImmediate(false);
		mainLayout.setWidth("100%");
		mainLayout.setHeight("100%");
		mainLayout.setMargin(false);
		
		// top-level component properties
		setWidth("100.0%");
		setHeight("100.0%");
		
		// horizontalLayoutUS
		horizontalLayoutUS = buildHorizontalLayoutUS();
		mainLayout.addComponent(horizontalLayoutUS);
		
		// horizontalLayoutCUType
		horizontalLayoutCUType = buildHorizontalLayoutCUType();
		mainLayout.addComponent(horizontalLayoutCUType);
		
		// horizontalLayout_1
		horizontalLayout_1 = buildHorizontalLayout_1();
		mainLayout.addComponent(horizontalLayout_1);
		
		// horizontalLayout_2
		horizontalLayout_2 = buildHorizontalLayout_2();
		mainLayout.addComponent(horizontalLayout_2);
		
		// horizontalLayout_3
		horizontalLayout_3 = buildHorizontalLayout_3();
		mainLayout.addComponent(horizontalLayout_3);
		
		// horizontalLayout_5
		horizontalLayout_5 = buildHorizontalLayout_5();
		mainLayout.addComponent(horizontalLayout_5);
		
		// horizontalLayout_4
		horizontalLayout_4 = buildHorizontalLayout_4();
		mainLayout.addComponent(horizontalLayout_4);
		
		return mainLayout;
	}

	@AutoGenerated
	private HorizontalLayout buildHorizontalLayoutUS() {
		// common part: create layout
		horizontalLayoutUS = new HorizontalLayout();
		horizontalLayoutUS.setImmediate(false);
		horizontalLayoutUS.setWidth("-1px");
		horizontalLayoutUS.setHeight("-1px");
		horizontalLayoutUS.setMargin(false);
		horizontalLayoutUS.setSpacing(true);
		
		// usLabel
		usLabel = new Label();
		usLabel.setImmediate(false);
		usLabel.setWidth("-1px");
		usLabel.setHeight("-1px");
		usLabel.setValue("Unit Server:");
		horizontalLayoutUS.addComponent(usLabel);
		
		// usSelected
		usSelected = new Label();
		usSelected.setImmediate(false);
		usSelected.setWidth("100.0%");
		usSelected.setHeight("-1px");
		usSelected.setValue("Unit Server Selected");
		horizontalLayoutUS.addComponent(usSelected);
		horizontalLayoutUS.setExpandRatio(usSelected, 1.0f);
		
		return horizontalLayoutUS;
	}

	@AutoGenerated
	private HorizontalLayout buildHorizontalLayoutCUType() {
		// common part: create layout
		horizontalLayoutCUType = new HorizontalLayout();
		horizontalLayoutCUType.setImmediate(false);
		horizontalLayoutCUType.setWidth("100.0%");
		horizontalLayoutCUType.setHeight("32px");
		horizontalLayoutCUType.setMargin(false);
		horizontalLayoutCUType.setSpacing(true);
		
		// cuTypeSelection
		cuTypeSelection = new NativeSelect();
		cuTypeSelection.setCaption("Class Type");
		cuTypeSelection.setImmediate(false);
		cuTypeSelection.setDescription("Class type selection");
		cuTypeSelection.setWidth("100.0%");
		cuTypeSelection.setHeight("-1px");
		horizontalLayoutCUType.addComponent(cuTypeSelection);
		
		// cuNewClass
		cuNewClass = new Button();
		cuNewClass.setCaption("CU Class..");
		cuNewClass.setImmediate(true);
		cuNewClass.setDescription("Create A New Class");
		cuNewClass.setWidth("-1px");
		cuNewClass.setHeight("-1px");
		horizontalLayoutCUType.addComponent(cuNewClass);
		horizontalLayoutCUType.setComponentAlignment(cuNewClass, new Alignment(
				6));
		
		return horizontalLayoutCUType;
	}

	@AutoGenerated
	private HorizontalLayout buildHorizontalLayout_1() {
		// common part: create layout
		horizontalLayout_1 = new HorizontalLayout();
		horizontalLayout_1.setImmediate(false);
		horizontalLayout_1.setWidth("100.0%");
		horizontalLayout_1.setHeight("-1px");
		horizontalLayout_1.setMargin(false);
		horizontalLayout_1.setSpacing(true);
		
		// cuIDLabel
		cuIDLabel = new Label();
		cuIDLabel.setImmediate(false);
		cuIDLabel.setWidth("-1px");
		cuIDLabel.setHeight("-1px");
		cuIDLabel.setValue("CU ID:");
		horizontalLayout_1.addComponent(cuIDLabel);
		
		// cuIDTextField
		cuIDTextField = new TextField();
		cuIDTextField.setImmediate(false);
		cuIDTextField.setDescription("Unique Device ID");
		cuIDTextField.setWidth("100.0%");
		cuIDTextField.setHeight("-1px");
		cuIDTextField.setInputPrompt("unique cu id");
		horizontalLayout_1.addComponent(cuIDTextField);
		horizontalLayout_1.setExpandRatio(cuIDTextField, 1.0f);
		
		return horizontalLayout_1;
	}

	@AutoGenerated
	private HorizontalLayout buildHorizontalLayout_2() {
		// common part: create layout
		horizontalLayout_2 = new HorizontalLayout();
		horizontalLayout_2.setImmediate(false);
		horizontalLayout_2.setWidth("530px");
		horizontalLayout_2.setHeight("93px");
		horizontalLayout_2.setMargin(false);
		
		// cuParamTextArea
		cuParamTextArea = new TextArea();
		cuParamTextArea.setCaption("Control Unit Load Parameter:");
		cuParamTextArea.setImmediate(false);
		cuParamTextArea.setWidth("100.0%");
		cuParamTextArea.setHeight("-1px");
		horizontalLayout_2.addComponent(cuParamTextArea);
		
		// checkBoxAutoLoad
		checkBoxAutoLoad = new CheckBox();
		checkBoxAutoLoad.setCaption("Auto load on unit server registration");
		checkBoxAutoLoad.setImmediate(false);
		checkBoxAutoLoad
				.setDescription("Create an instance as soon the Unit Server registers");
		checkBoxAutoLoad.setWidth("-1px");
		checkBoxAutoLoad.setHeight("-1px");
		horizontalLayout_2.addComponent(checkBoxAutoLoad);
		horizontalLayout_2.setComponentAlignment(checkBoxAutoLoad,
				new Alignment(6));
		
		return horizontalLayout_2;
	}

	@AutoGenerated
	private HorizontalLayout buildHorizontalLayout_3() {
		// common part: create layout
		horizontalLayout_3 = new HorizontalLayout();
		horizontalLayout_3.setImmediate(false);
		horizontalLayout_3.setWidth("100.0%");
		horizontalLayout_3.setHeight("-1px");
		horizontalLayout_3.setMargin(false);
		
		// verticalLayout_2
		verticalLayout_2 = buildVerticalLayout_2();
		horizontalLayout_3.addComponent(verticalLayout_2);
		
		return horizontalLayout_3;
	}

	@AutoGenerated
	private VerticalLayout buildVerticalLayout_2() {
		// common part: create layout
		verticalLayout_2 = new VerticalLayout();
		verticalLayout_2.setImmediate(false);
		verticalLayout_2.setWidth("100.0%");
		verticalLayout_2.setHeight("-1px");
		verticalLayout_2.setMargin(false);
		
		// tableDriverSpecifications
		tableDriverSpecifications = new Table();
		tableDriverSpecifications.setCaption("Driver specifications");
		tableDriverSpecifications.setImmediate(false);
		tableDriverSpecifications.setWidth("100.0%");
		tableDriverSpecifications.setHeight("-1px");
		verticalLayout_2.addComponent(tableDriverSpecifications);
		
		// horizontalLayout_6
		horizontalLayout_6 = buildHorizontalLayout_6();
		verticalLayout_2.addComponent(horizontalLayout_6);
		
		return verticalLayout_2;
	}

	@AutoGenerated
	private HorizontalLayout buildHorizontalLayout_6() {
		// common part: create layout
		horizontalLayout_6 = new HorizontalLayout();
		horizontalLayout_6.setImmediate(false);
		horizontalLayout_6.setWidth("-1px");
		horizontalLayout_6.setHeight("-1px");
		horizontalLayout_6.setMargin(true);
		horizontalLayout_6.setSpacing(true);
		
		// buttonAddDriverSpec
		buttonAddDriverSpec = new Button();
		buttonAddDriverSpec.setCaption("Add");
		buttonAddDriverSpec.setImmediate(true);
		buttonAddDriverSpec.setWidth("-1px");
		buttonAddDriverSpec.setHeight("-1px");
		horizontalLayout_6.addComponent(buttonAddDriverSpec);
		horizontalLayout_6.setComponentAlignment(buttonAddDriverSpec,
				new Alignment(33));
		
		// buttonRemoveDriverSpec
		buttonRemoveDriverSpec = new Button();
		buttonRemoveDriverSpec.setCaption("Remove");
		buttonRemoveDriverSpec.setImmediate(true);
		buttonRemoveDriverSpec.setWidth("-1px");
		buttonRemoveDriverSpec.setHeight("-1px");
		horizontalLayout_6.addComponent(buttonRemoveDriverSpec);
		horizontalLayout_6.setComponentAlignment(buttonRemoveDriverSpec,
				new Alignment(33));
		
		return horizontalLayout_6;
	}

	@AutoGenerated
	private HorizontalLayout buildHorizontalLayout_5() {
		// common part: create layout
		horizontalLayout_5 = new HorizontalLayout();
		horizontalLayout_5.setImmediate(false);
		horizontalLayout_5.setWidth("100.0%");
		horizontalLayout_5.setHeight("-1px");
		horizontalLayout_5.setMargin(false);
		
		// verticalLayout_4
		verticalLayout_4 = buildVerticalLayout_4();
		horizontalLayout_5.addComponent(verticalLayout_4);
		
		return horizontalLayout_5;
	}

	@AutoGenerated
	private VerticalLayout buildVerticalLayout_4() {
		// common part: create layout
		verticalLayout_4 = new VerticalLayout();
		verticalLayout_4.setImmediate(false);
		verticalLayout_4.setWidth("100.0%");
		verticalLayout_4.setHeight("-1px");
		verticalLayout_4.setMargin(false);
		
		// table_attributes
		table_attributes = new Table();
		table_attributes.setCaption("Attribute specifications");
		table_attributes.setImmediate(false);
		table_attributes.setWidth("100.0%");
		table_attributes.setHeight("100.0%");
		verticalLayout_4.addComponent(table_attributes);
		
		// horizontalLayout_8
		horizontalLayout_8 = buildHorizontalLayout_8();
		verticalLayout_4.addComponent(horizontalLayout_8);
		
		return verticalLayout_4;
	}

	@AutoGenerated
	private HorizontalLayout buildHorizontalLayout_8() {
		// common part: create layout
		horizontalLayout_8 = new HorizontalLayout();
		horizontalLayout_8.setImmediate(false);
		horizontalLayout_8.setWidth("-1px");
		horizontalLayout_8.setHeight("-1px");
		horizontalLayout_8.setMargin(true);
		horizontalLayout_8.setSpacing(true);
		
		// buttonAddAttr
		buttonAddAttr = new Button();
		buttonAddAttr.setCaption("Add");
		buttonAddAttr.setImmediate(true);
		buttonAddAttr.setDescription("Add a attribute for the CU");
		buttonAddAttr.setWidth("-1px");
		buttonAddAttr.setHeight("-1px");
		horizontalLayout_8.addComponent(buttonAddAttr);
		horizontalLayout_8.setComponentAlignment(buttonAddAttr, new Alignment(
				33));
		
		// buttonRemAttr
		buttonRemAttr = new Button();
		buttonRemAttr.setCaption("Remove");
		buttonRemAttr.setImmediate(true);
		buttonRemAttr.setDescription("Remove an attribute");
		buttonRemAttr.setWidth("-1px");
		buttonRemAttr.setHeight("-1px");
		horizontalLayout_8.addComponent(buttonRemAttr);
		horizontalLayout_8.setComponentAlignment(buttonRemAttr, new Alignment(
				33));
		
		return horizontalLayout_8;
	}

	@AutoGenerated
	private HorizontalLayout buildHorizontalLayout_4() {
		// common part: create layout
		horizontalLayout_4 = new HorizontalLayout();
		horizontalLayout_4.setImmediate(false);
		horizontalLayout_4.setWidth("100.0%");
		horizontalLayout_4.setHeight("-1px");
		horizontalLayout_4.setMargin(false);
		horizontalLayout_4.setSpacing(true);
		
		// buttonSave
		buttonSave = new Button();
		buttonSave.setCaption("Save");
		buttonSave.setImmediate(true);
		buttonSave.setDescription("Save");
		buttonSave.setWidth("-1px");
		buttonSave.setHeight("-1px");
		horizontalLayout_4.addComponent(buttonSave);
		horizontalLayout_4.setComponentAlignment(buttonSave, new Alignment(34));
		
		// buttonCancel
		buttonCancel = new Button();
		buttonCancel.setCaption("Cancel");
		buttonCancel.setImmediate(true);
		buttonCancel.setWidth("-1px");
		buttonCancel.setHeight("-1px");
		horizontalLayout_4.addComponent(buttonCancel);
		horizontalLayout_4.setComponentAlignment(buttonCancel,
				new Alignment(33));
		
		return horizontalLayout_4;
	}

}
